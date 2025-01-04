#ifndef STRUCT_H
#define STRUCT_H
#include <opencv2/opencv.hpp>
#include <iostream>


enum 
{
    LOSS = 0,
    DROP,
    FIND
};

struct detectInfo
{
    cv::Point2f center_point;
    int class_id;
    float conf;
    float area;
    std::chrono::time_point<std::chrono::steady_clock> time_stamp;
}; 

struct imgInfo
{
    cv::Mat img;
    std::vector<detectInfo> detectInfos;
};

struct imgcache
{
    cv::Point2f center_point;
    int class_id;
    float area;
    int State = LOSS;
    std::chrono::time_point<std::chrono::steady_clock> time_stamp;
};

struct objectInfo
{
    int class_id;
    double setPoint;
    std::chrono::time_point<std::chrono::steady_clock> time_stamp;
};

#endif // STRUCT_H