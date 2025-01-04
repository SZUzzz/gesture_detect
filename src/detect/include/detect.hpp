#ifndef DETECT_HPP
#define DETECT_HPP
#include <opencv2/opencv.hpp>
#include "OpenvinoInfer.h"
#include "Struct.h"
#include "pid.hpp"
#include "param.h"


#define XML_PATH "/home/nuc/Documents/zz/gesture_detect/model/bestwithno.xml"
#define BIN_PATH "/home/nuc/Documents/zz/gesture_detect/model/bestwithno.bin"
class Gesture 
{
public:
    explicit Gesture();
    ~Gesture();
    void detect(cv::Mat &img); 
    void track(detectInfo &d_i_);
    void updateState(imgInfo &i_i);
    void findBestObject(std::vector<std::vector<imgcache>> i_cs, objectInfo &o_i);


private:
    enum 
    {
        No_= 0,
        First_,
        One_,
        Two_,
        Three_,
        Four_,
        Five_,
    };
    OpenvinoInfer *openvino_infer_;
    bool startup_ = true;
    imgInfo last_i_i;
    int img_w;
    int img_h;
    long long find_count[7] = {0};
    long long loss_count[7] = {0};
    int State[7];
    PIDController pid;
    objectInfo last_o_i;
    std::vector<std::vector<imgcache>> i_cs;
    bool detect_init =false;
};

#endif //DETECT_HPP