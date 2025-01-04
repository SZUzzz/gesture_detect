#include "detect.hpp"

Gesture::Gesture() : pid(KP, KI, KD)
{
    std::map<std::string, std::string> path;
    path.insert(std::pair<std::string, std::string>("XML",XML_PATH));
    path.insert(std::pair<std::string, std::string>("BIN",BIN_PATH));
    this->openvino_infer_ = new OpenvinoInfer(path);
    i_cs.resize(7);
    for(size_t i = 0; i < 7; i++)
    {
        State[i] = LOSS;
    }
}

Gesture::~Gesture()
{
    delete this->openvino_infer_;
}

void Gesture::track(detectInfo &d_i_) {
    double output = pid.update((double)d_i_.center_point.x, img_w/2);
    std::cout << "------correct: " << output <<std::endl;
}

void Gesture::updateState(imgInfo &i_i)
{   
    for(size_t i = 0; i < i_cs.size(); i++)
    {
        i_cs[i].clear();
    }

    for(size_t i = 0; i < i_i.detectInfos.size(); i++)
    {
        imgcache i_c;
        int id = i_i.detectInfos[i].class_id;
        switch (id)
        {
        case No_:
            // i_c.center_point = i_i.detectInfos[i].center_point;
            // i_c.class_id = id;
            // i_c.time_stamp = i_i.detectInfos[i].time_stamp;
            // i_c.area = i_i.detectInfos[i].area;
            // i_cs[No_].emplace_back(i_c);
            break;
        case First_:
            i_c.center_point = i_i.detectInfos[i].center_point;
            i_c.class_id = id;
            i_c.time_stamp = i_i.detectInfos[i].time_stamp;
            i_c.area = i_i.detectInfos[i].area;
            i_cs[First_].emplace_back(i_c);
            break;
        case One_:
            i_c.center_point = i_i.detectInfos[i].center_point;
            i_c.class_id = id;
            i_c.time_stamp = i_i.detectInfos[i].time_stamp;
            i_c.area = i_i.detectInfos[i].area;
            i_cs[One_].emplace_back(i_c);
            break;
        case Two_:
            i_c.center_point = i_i.detectInfos[i].center_point;
            i_c.class_id = id;
            i_c.time_stamp = i_i.detectInfos[i].time_stamp;
            i_c.area = i_i.detectInfos[i].area;
            i_cs[Two_].emplace_back(i_c);
            break;
        case Three_:
            i_c.center_point = i_i.detectInfos[i].center_point;
            i_c.class_id = id;
            i_c.time_stamp = i_i.detectInfos[i].time_stamp;
            i_c.area = i_i.detectInfos[i].area;
            i_cs[Three_].emplace_back(i_c);
            break;
        case Four_:
            i_c.center_point = i_i.detectInfos[i].center_point;
            i_c.class_id = id;
            i_c.time_stamp = i_i.detectInfos[i].time_stamp;
            i_c.area = i_i.detectInfos[i].area;
            i_cs[Four_].emplace_back(i_c);
            break;
        case Five_:
            i_c.center_point = i_i.detectInfos[i].center_point;
            i_c.class_id = id;
            i_c.time_stamp = i_i.detectInfos[i].time_stamp;
            i_c.area = i_i.detectInfos[i].area;
            i_cs[Five_].emplace_back(i_c);
            break;
        default:
            break;
        }
    }

    for (size_t i = 0; i < i_cs.size(); i++)
    {
        if(i == 0)
        {
            continue;
        }
        if(i_cs[i].empty() && State[i] == LOSS)
        {
            find_count[i] = 0;
            if(i == 6) {
                detect_init = false;
            }
            continue;
        } else if (i_cs[i].empty() && State[i] != LOSS) {
            loss_count[i]++;
            find_count[i] = 0;
            State[i] = DROP;
            if(loss_count[i] > 5) {
                State[i] = LOSS;
            }
        } else {
            loss_count[i] = 0;
            find_count[i]++;
        }
        if(find_count[i] > 2) {
            State[i] = FIND;
            sort(i_cs[i].begin(), i_cs[i].end(), [](imgcache &a, imgcache &b) {
                return a.area > b.area;
            });
        }
    }
}

void Gesture::findBestObject(std::vector<std::vector<imgcache>> i_cs, objectInfo &o_i)
{
    for(size_t i = 0; i < i_cs.size(); i++)
    {
        if(i == 0)
        {
            continue;
        }
        std::cout  << "-----------------i: " << i << "State" << State[i] <<std::endl;
        if(i_cs[i].empty() || State[i] != FIND)
        {
            continue;
        }
        if(!detect_init) 
        {
            last_o_i.class_id = i;
            detect_init = true;
        }
        if(i != last_o_i.class_id) {
            continue;
        } else {
            o_i.class_id = i;
            o_i.setPoint = i_cs[i][0].center_point.x;
            o_i.time_stamp = i_cs[i][0].time_stamp;
        }
    }
    last_o_i = o_i;
}

void Gesture::detect(cv::Mat &img_) 
{
    imgInfo i_i;
    i_i.img = img_.clone();
    img_w = img_.cols;
    img_h = img_.rows;
    this->openvino_infer_->infer(img_, this->startup_);
    // if (this->startup_) {
    //     this->startup_ = false;
    //     this->last_i_i.img = img_.clone();
    //     return;
    // }
    this->openvino_infer_->fitRec(this->openvino_infer_->objects, 
                    cv::Size2d(img_.cols, img_.rows),
                    cv::Size2d(640.0,640.0));
    i_i.detectInfos.resize(this->openvino_infer_->objects.size());
    for( int i = 0; i < this->openvino_infer_->objects.size(); i++)
    {
        for( int p = 0; p < 4; p++)
        {
            cv::line(img_, 
                    this->openvino_infer_->objects[i].landmarks[p], 
                    this->openvino_infer_->objects[i].landmarks[(p+1)%4], 
                    cv::Scalar(34, 123, 32), 
                    2);
        }
        cv::putText(img_,
                    std::to_string(this->openvino_infer_->objects[i].class_id), 
                    this->openvino_infer_->objects[i].landmarks[0], 
                    cv::FONT_HERSHEY_SIMPLEX, 
                    0.5, 
                    cv::Scalar(0, 255, 0), 
                    2);
        cv::putText(img_,
                    "CONF: "+std::to_string(this->openvino_infer_->objects[i].conf), 
                    cv::Point(this->openvino_infer_->objects[i].landmarks[0].x+20, 
                    this->openvino_infer_->objects[i].landmarks[0].y), 
                    cv::FONT_HERSHEY_SIMPLEX, 
                    0.5, 
                    cv::Scalar(0, 255, 0), 
                    2);
        cv::Point2f center_point;
        center_point.x = ((this->openvino_infer_->objects[i].landmarks[0].x + this->openvino_infer_->objects[i].landmarks[2].x) / 2 + 
        (this->openvino_infer_->objects[i].landmarks[1].x + this->openvino_infer_->objects[i].landmarks[3].x) / 2) / 2;
        center_point.y = ((this->openvino_infer_->objects[i].landmarks[0].y + this->openvino_infer_->objects[i].landmarks[2].y) / 2 + 
        (this->openvino_infer_->objects[i].landmarks[1].y + this->openvino_infer_->objects[i].landmarks[3].y) / 2) / 2;
        cv::circle(img_, center_point, 3, cv::Scalar(255,0,0), 3, 8, 0);
        detectInfo d_i;
        d_i.center_point = center_point;
        d_i.class_id = this->openvino_infer_->objects[i].class_id;
        d_i.conf = this->openvino_infer_->objects[i].conf;
        d_i.area = this->openvino_infer_->objects[i].width * this->openvino_infer_->objects[i].length;
        i_i.detectInfos.emplace_back(d_i);
    }
    objectInfo o_i;
    this->updateState(i_i);
    this->findBestObject(i_cs, o_i);
    this->last_i_i = std::move(i_i);
}