#include <iostream>
#include <thread>
#include <opencv2/opencv.hpp>
#include "hik_camera.h"
#include "detect.hpp"
#include "serialport.h"
#include "Record.h"
#include "Struct.h"
#include "fileToolkit.hpp"
#include "param.h"

volatile bool isrun = true;
volatile int process_index = 0;
volatile int receive_index = 0;

std::mutex m;
cv::Mat img;
cv::Mat img_show;
SerialPort sp_;
CarData cd_;
VisionData vd_;

#define SHOW_
// #define RECORD_

std::chrono::time_point<std::chrono::steady_clock> start, end;
filetoolkit::FILETOOLKIT file("../config/param.txt");
// void receive()
// {
//     static HikCameraManager camera_manager;
//     camera_manager.init_manager_for_one_camera();
//     HikCamera* test_camera = camera_manager.get_hik_camera(0);
//     test_camera->set_float_value("ExposureTime", 13000);
//     test_camera->set_float_value("Gain", 6.0);
//     cv::Mat src;
//     // sp_.init();
//     while (isrun)
//     {
//         test_camera->get_image(src);
//         while((receive_index - process_index) > 0 && isrun);
//         m.lock();
//         img = src;
//         // sp_.recieve2(cd_);
//         m.unlock();
//         receive_index++;
//         std::cout << "receive_index: " << receive_index << std::endl;
//     }
// }

void receive() {
    cv::VideoCapture capture(0);
    if(!capture.isOpened()) {
        std::cout << "open camera fail !" << std::endl;
        return;
    }
    cv::Mat src;
    while (isrun)
    {
        capture >> src;
        while((receive_index - process_index) > 0 && isrun);
        m.lock();
        img = src;
        // sp_.recieve2(cd_);
        m.unlock();
        receive_index++;
        std::cout << "receive_index: " << receive_index << std::endl;
    }
    
}

void process()
{
    cv::Mat img_p;
    Gesture gt;
    while (isrun)
    {
        start = std::chrono::steady_clock::now();
        while((receive_index - process_index) < 1 && isrun);
        m.lock();
        img_p = img.clone();
        std::cout << "process_index: " << process_index << std::endl;
        m.unlock();
        process_index++;
        gt.detect(img_p);

#ifdef SHOW_
        m.lock();
        if(img_p.empty())
        {
            m.unlock();
            continue;
        }
        img_show = img_p.clone();
        m.unlock();
#endif
        // end = std::chrono::steady_clock::now();
        // std::chrono::duration<double> elapsed_seconds = end - start;
        // std::cout << "elapsed time: " << elapsed_seconds.count() << "s\n";
        // sp_.send(vd_);
        file.isFileModifiedByTime("../config/param.txt");
        file.updateFile("../config/param.txt");
    }
}

void show()
{
    cv::namedWindow("Img", cv::WINDOW_NORMAL | cv::WINDOW_KEEPRATIO);
    while (isrun)
    {
        if(img_show.empty())
            continue;
        cv::imshow("Img", img_show);
        cv::waitKey(1);
    }
}

void record()
{
    cv::Mat img_c;
    Record record;
    record.Video_init();
    while (1)
    {
        m.lock();
        img_c = img.clone();
        record.Video(img_c);
        m.unlock();
        if (img_c.empty() != true) {record.Video(img_c);}
    }
}

int main()
{
    std::thread receive_thread(receive);
    std::thread process_thread(process);
#ifdef SHOW_
    std::thread show_thread(show);
#endif
#ifdef RECORD_
    std::thread record_thread(record);
#endif
    if(receive_thread.joinable())
        receive_thread.join();    
    if(process_thread.joinable())
        process_thread.join();

#ifdef SHOW_
    if(show_thread.joinable())
        show_thread.join();
#endif
#ifdef RECORD_
    if(record_thread.joinable())
        record_thread.join();
#endif
    return 0;
}