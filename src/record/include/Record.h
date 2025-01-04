#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

class Record
{
private:
    char img_key_;
    std::string img_str_head_ = "../Record_Image/out";
    int take_img_ = 0;
    std::string img_str_tail_ = ".png";
    std::string img_str_name_;
    bool img_get_ = false;
    cv::VideoWriter video_writer_;
    std::string video_str_head_ = "/home/nuc/zz/gesture_detect/src/record/Record_Video/";
    std::string video_str_time_;
    std::string video_str_tail_ = ".avi";
    std::string video_str_name_;
    bool video_recording_ = false;
public:
    void Video_init();
    void Video(cv::Mat img);
    void Image(cv::Mat img, char img_key);
};

