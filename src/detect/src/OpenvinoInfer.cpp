#include "OpenvinoInfer.h"
using namespace cv;
using namespace std;


OpenvinoInfer::OpenvinoInfer(std::map<std::string, std::string> &path) {
    input_shape = {1, static_cast<unsigned long>(IMAGE_HEIGHT), static_cast<unsigned long>(IMAGE_WIDTH), 3};
    std::cout << "-----" << path["XML"] << " " << path["BIN"] <<std::endl;
    this->model = this->core.read_model(path["XML"],path["BIN"]);
    // Step . Inizialize Preprocessing for the model
    ppp = new ov::preprocess::PrePostProcessor(model);
    // Specify input image format
    ppp->input().
        tensor().
        set_element_type(ov::element::u8).
        set_layout("NHWC").
        set_color_format(ov::preprocess::ColorFormat::BGR); 
    //NHWC:batchsize,height,width,channels
    // Specify preprocess pipeline to input image without resizing
    ppp->input().preprocess().
        convert_element_type(ov::element::f32).
        convert_color(ov::preprocess::ColorFormat::RGB).
        scale({255., 255., 255.});
    //  Specify model's input layout
    ppp->input().
        model().
        set_layout("NCHW");
    // Specify output results format
    // 确认输出数量
    if (model->outputs().size() > 1) {
        // 如果需要处理多个输出，请确保选择所需的输出节点
        std::cerr << "Warning: Model has multiple outputs. Defaulting to the first output." << std::endl;
        auto output = model->output(0);
        ppp->output(output.
            get_any_name()).
            tensor().
            set_element_type(ov::element::f32);
    } else {
        ppp->output().
            tensor().
            set_element_type(ov::element::f32);
        std::cerr << "Warning: Model has only one outputs." << std::endl;
    }
    // Embed above steps in the graph
    model = ppp->build();

    compiled_model = core.compile_model(model, device);
    compiled_model_next =core.compile_model(model,device);
    this->infer_request = compiled_model.create_infer_request();

    // this->infer_requests_.insert(std::pair<bool, ov::InferRequest>(1,compiled_model.create_infer_request()));
    // this->infer_requests_.insert(std::pair<bool, ov::InferRequest>(0,compiled_model_next.create_infer_request()));
}


/**
 * 在resize是像素的横纵比发生变化，故用letterbox
 * @param src
 * @param dst_size
 * @return
 */
cv::Mat OpenvinoInfer::letterBox(cv::Mat &src, const cv::Size2d &dst_size) {
        int w = src.cols;
        int h = src.rows;

        double r_w = (double)dst_size.width / w;
        double r_h = (double)dst_size.height / h;
        double r = std::min(r_w, r_h);

        int new_w = (int)(w * r);
        int new_h = (int)(h * r);

        cv::Mat resized_image;

        cv::resize(src, resized_image, cv::Size(new_w, new_h), cv::INTER_CUBIC);

        cv::Mat canvas(dst_size, CV_8UC3, cv::Scalar(128, 128, 128));

        this->dx = (int)((dst_size.width - new_w) / 2.);
        this->dy  =(int)((dst_size.height - new_h) / 2.);

        resized_image.copyTo(canvas(cv::Rect(dx, dy, new_w, new_h)));
        // cv::imshow("resized_image", canvas);
        // cv::waitKey(1);

        return canvas;
}

/**
 * 在推理时图像被resize，最终的关键点需要变回原本比例的
 * @param bboxes
 * @param ori_size
 * @param now_size
 */
void OpenvinoInfer::fitRec(std::vector<OpenvinoInfer::Object> &bboxes,
                           cv::Size2d ori_size,
                           cv::Size2d now_size)
{
    double scale = std::max((double)ori_size.width / now_size.width, (double)ori_size.height / now_size.height);
    for (auto &bbox : bboxes) {
        for (int i = 0 ; i < 4 ; i++) {
            bbox.landmarks[i].x = (bbox.landmarks[i].x - now_size.width / 2) * scale + ori_size.width / 2;
            bbox.landmarks[i].y = (bbox.landmarks[i].y - now_size.height / 2) * scale + ori_size.height / 2;
        }
    }
}

void OpenvinoInfer::infer(cv::Mat img, const bool &startup){
    objects.clear();
    ious.clear();
    // Step 3. Read input image
    // resize image
    img = letterBox(img, cv::Size2d(640.0,640.0));
    // Step 5. Create tensor from image
    int rows = img.rows;
    int cols = img.cols;

    uchar* input_data = (uchar *)img.data; // 创建一个新的float数组
    if(input_data == nullptr)
    {
        cerr<<"Error input empty"<<endl;
        exit(-1);
    }

    ov::Tensor input_tensor = ov::Tensor(compiled_model.input().get_element_type(), compiled_model.input().get_shape(), input_data);
    // Step 6. Create an infer request for model inference

    // if (startup) {
    //     infer_requests_[1].set_input_tensor(input_tensor);
    //     infer_requests_[1].start_async();
    //     return;
    // }
    // infer_requests_[0].set_input_tensor(input_tensor);
    // infer_requests_[0].start_async();
    // infer_requests_[1].wait();

    infer_request.set_input_tensor(input_tensor);
    double ta = cv::getTickCount();
    infer_request.infer();
    double tb = cv::getTickCount();

    //Step 7. Retrieve inference results

    // Step 8. get result
    // -------- Step 8. Post-process the inference result -----------
    // auto output = infer_requests_[1].get_output_tensor(0);
    auto output = infer_request.get_output_tensor(0);
    ov::Shape output_shape = output.get_shape();
    // 1*25200*11 Matrix
    cv::Mat output_buffer(output_shape[1], output_shape[2], CV_32F, output.data());
    float conf_threshold = 0.64;
    float nms_threshold = 0.45;
    std::vector<cv::Rect> boxes;
    std::vector<int> class_ids;
    std::vector<float> class_scores;
    std::vector<float> confidences;
    for (int i = 0; i < output_buffer.rows; i++) {
        //通过置信度阈值筛选
        float confidence = output_buffer.at<float>(i, 4);
        confidence = sigmoid(confidence);
        if (confidence < conf_threshold)
        {
            continue;
        }
        float x = output_buffer.at<float>(i, 0);
        float y = output_buffer.at<float>(i, 1);
        float w = output_buffer.at<float>(i, 2);
        float h = output_buffer.at<float>(i, 3);
        cv::Mat classes_scores = output_buffer.row(i).colRange(4, 11); //num

        cv::Point class_id;
        int _class_id;
        double score_num;
        cv::minMaxLoc(classes_scores, NULL, &score_num, NULL, &class_id);
        _class_id = class_id.x;

        std::vector<cv::Point2f> points;
        // points为左上顺时针
        points.push_back(cv::Point2f(x-w/2, y-h/2));
        points.push_back(cv::Point2f(x+w/2, y-h/2));
        points.push_back(cv::Point2f(x+w/2, y+h/2));
        points.push_back(cv::Point2f(x-w/2, y+h/2));


        // Find the minimum and maximum x and y coordinates
        float min_x = points[0].x;
        float max_x = points[0].x;
        float min_y = points[0].y;
        float max_y = points[0].y;

        for (int i = 1; i < points.size(); i++)
        {
            if (points[i].x < min_x)
                min_x = points[i].x;
            if (points[i].x > max_x)
                max_x = points[i].x;
            if (points[i].y < min_y)
                min_y = points[i].y;
            if (points[i].y > max_y)
                max_y = points[i].y;
        }
        Object obj;

        // Create the rectangle
        cv::Rect rect(min_x, min_y, max_x - min_x, max_y - min_y);
        float w_ = max_x - min_x;
        float h_ = max_y - min_y;
        obj.rect = rect;
        obj.landmarks[0].x = min_x;
        obj.landmarks[0].y = min_y;
        obj.landmarks[1].x = x+w_/2;
        obj.landmarks[1].y = y-h_/2;
        obj.landmarks[2].x = max_x;
        obj.landmarks[2].y = max_y;
        obj.landmarks[3].x = x-w_/2;
        obj.landmarks[3].y = y+h_/2;
        obj.conf =confidence;
        obj.class_id = _class_id;
        obj.width = w;
        obj.length = h;
        objects.push_back(obj);
        boxes.push_back(rect);
        confidences.push_back(score_num);
    }
    
    // NMS
    std::vector<int> indices;
    cv::dnn::NMSBoxes(boxes, confidences, conf_threshold, nms_threshold, indices);
    int index = 0, index_indices = 0;
    for (auto object = objects.begin(); object != objects.end();) {
        if (indices.size() == 0) break;
        if (index != indices[index_indices]) {
            object = objects.erase(object);
        } else {
            ++object;
            ++index_indices;
        }
        index++;
    }

    // auto change = infer_requests_[1];
    // infer_requests_[1] = this->infer_requests_[0];
    // infer_requests_[0] = change;

    return;
}
