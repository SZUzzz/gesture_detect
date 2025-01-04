#include "Record.h"
using namespace cv;
using namespace std;

void Record::Video_init(){
    chrono::time_point<chrono::system_clock> now= chrono::system_clock::now();// 获取当前系统时间
    time_t now_time = chrono::system_clock::to_time_t(now);// 将时间转换为可打印的格式
    tm* local_time = std::localtime(&now_time);// 将时间转换为本地时间
    stringstream time;
    time << put_time(local_time, "%Y_%m_%d_%H:%M:%S");
    video_str_time_ = time.str();
    video_str_name_ = video_str_head_ + video_str_time_ + video_str_tail_;
    cout << video_str_name_ << endl;
    video_writer_.open(video_str_name_, VideoWriter::fourcc('M', 'J', 'P', 'G'), 60, Size(1440, 1080), true);

}
void Record::Video(Mat img){
    if (video_recording_ == false) {
        cout << "video recording:" << video_str_name_ << endl;
        video_recording_ = true;
    }
    video_writer_ << img;
}
void Record::Image(Mat img, char img_key){
    if (img_key=='i' || img_key=='I') {
        take_img_++;
        img_str_name_ = img_str_head_ + to_string(take_img_) + img_str_tail_;
        img_get_ = imwrite(img_str_name_, img);
        if (img_get_) {cout << "save image succeed:" << img_str_name_ << endl;}
        else {cout << "save image failed:" << img_str_name_ << endl;}
    }
}


