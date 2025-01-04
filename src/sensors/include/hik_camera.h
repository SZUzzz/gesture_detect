#ifndef _HIK_CAMERA_
#define _HIK_CAMERA_
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <string>
#include "MvCameraControl.h"
#include <opencv2/opencv.hpp>

enum HIK_Camera_State{
    IDLE,
    OPENED,
    GRABBING,
    WRONG_STATE
};

class HikCamera{
public:
    HikCamera(){}
    ~HikCamera();

    int init_camera(MV_CC_DEVICE_INFO *camera_device);

    void set_param();
    bool set_resolution(int max_width, int roi_width, int roi_width_offset, int max_height, int roi_height, int roi_height_offset);

    int check_camera_state();
    
    int create_handle();

    int open_camera();
    void close_camera();

    void set_trigger_mode();

    unsigned int get_int_value(const char *Key);
    float get_float_value(const char *Key);
    unsigned int get_enum_value(const char *Key);
    bool get_bool_value(const char *Key);
    std::string get_string_value(const char *Key);

    bool set_int_value(const char *Key, unsigned int iValue);
    bool set_enum_value(const char *Key, unsigned int Value);
    bool set_float_value(const char *Key, float fValue);
    bool set_bool_value(const char *Key, bool bValue);
    bool set_string_value(const char *Key, char* strValue);

    int grab_image();

    void get_image(cv::Mat &M);

public:

    int no_data_times = 0;
    std::string m_name = "camera";
    std::string m_ID = "none";
    void* handle = nullptr;

    int now_mode = -1;
    int now_color = -1;

private:
    int nRet = MV_OK;
    MV_CC_DEVICE_INFO* pDeviceInfo = nullptr;
    MVCC_INTVALUE stParam;
    unsigned char* pData = nullptr;
    unsigned char *pDataForBGR = nullptr;
    MV_FRAME_OUT_INFO_EX stImageInfo = {0};
    int camera_state = HIK_Camera_State::IDLE;
    unsigned int nDataSize = 0;
    
};

class HikCameraManager{
public:

    explicit HikCameraManager();
    ~HikCameraManager() {
    if(cameras != nullptr){
        if(cameras_size==1){
            delete cameras;
        }
        else{
            delete[] cameras;
        }
        cameras = nullptr;
    }
    }

    void init_manager_for_one_camera();
    bool init_just_one_camera(HikCamera &hik_camera);
    bool init_one_camera_in_ID(HikCamera &hik_camera, std::string device_ID);
    int enum_device();

    HikCamera* get_hik_camera(int index);

private:
    int nRet = MV_OK;
    MV_CC_DEVICE_INFO_LIST stDeviceList;
    HikCamera* cameras = nullptr;
    int cameras_size=0;
};

#endif