#ifndef _SERIALPORT_H_  // 头文件保护宏，避免重复包含
#define _SERIALPORT_H_

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/serial.h>
#include <memory.h>
#include <opencv2/opencv.hpp>
#include "CRC_Check.h"
// subscibtion
struct VisionData
{
    uint8_t header = 0xA5;
    
    uint16_t CRC16 = 0;
}__attribute__((packed));

// publish
struct CarData
{
    uint8_t header = 0xA5;

    uint16_t CRC16;
}__attribute__((packed));


class SerialPort
{
public:
    explicit SerialPort();
    ~SerialPort();
    bool init();
    bool recieve(CarData& cardata);
    bool recieve2(CarData& cardata);
    void send(VisionData &vd);
    void close_port();
    int result;
    uint8_t mode;
    bool check_data(VisionData& vd);
    bool check_data(const CarData& cardata);
private:
    void set_brate();
    int set_bit();
    int fd;
    int boudrate, databits, stopbits, parity;
    unsigned char rdata[8192];
    unsigned char Tdata[80];
};

#endif
