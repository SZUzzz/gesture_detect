#ifndef OPENVINO_TEST_OPENVINOINFER_H
#define OPENVINO_TEST_OPENVINOINFER_H

#include <opencv2/opencv.hpp>
#include <openvino/openvino.hpp>
#include <vector>
#include <iostream>

#define device "GPU"
enum 
{
    First_ = 0,
    One_,
    Two_,
    Three_,
    Four_,
    Five_,
};
class OpenvinoInfer {
    struct Object
    {
        cv::Rect_<float> rect;
        cv::Vec<cv::Point2d,4> landmarks;
        double width;
        double length;
        int class_id;
        float conf;
    };
public:
    OpenvinoInfer() {};
    explicit OpenvinoInfer(std::map<std::string, std::string> &path);
    ~OpenvinoInfer() {
        delete ppp;
    }
    cv::Mat letterBox(cv::Mat &src, const cv::Size2d &dst_size);
    void fitRec(std::vector<Object> &bboxes,
                           cv::Size2d ori_size,
                           cv::Size2d now_size);
    void infer(cv::Mat img, const bool &startup);

    double sigmoid(double x) {
        if(x>0)
            return 1.0 / (1.0 + exp(-x));
        else
            return exp(x) / (1.0 + exp(x));
    }

    double cal_iou(const cv::Rect& r1, const cv::Rect& r2)
    {
        float x_left = std::fmax(r1.x, r2.x);
        float y_top = std::fmax(r1.y, r2.y); 
        float x_right = std::fmin(r1.x + r1.width, r2.x + r2.width);
        float y_bottom = std::fmin(r1.y + r1.height, r2.y + r2.height);

        if (x_right < x_left || y_bottom < y_top) {
            return 0.0; 
        }

        double in_area = (x_right - x_left) * (y_bottom - y_top);
        double un_area = r1.area() + r2.area() - in_area; 

        return in_area / un_area;
    }

    double meaning(float x, int len){
        if(len == 0) ans = x;
        else{
            ans = (len * ans + x) / (len+1);
        }
        return ans;
    }

    std::vector<Object> objects;
    double ans;
    std::vector<double> ious;
    std::shared_ptr<ov::Model> model;
    ov::Core core;
    ov::preprocess::PrePostProcessor *ppp;
    ov::CompiledModel compiled_model;
    ov::CompiledModel compiled_model_next;

    ov::Shape input_shape;
    ov::InferRequest infer_request, compiled_model_next_;
    std::map<bool, ov::InferRequest> infer_requests_;

    int IMAGE_HEIGHT = 640;
    int IMAGE_WIDTH = 640;
    float dx = 0.0;
    float dy = 0.0;
};

#endif //OPENVINO_TEST_OPENVINOINFER_H;
