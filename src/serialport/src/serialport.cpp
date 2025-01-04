#include "serialport.h"
//参考博客： https://blog.csdn.net/lvdepeng123/article/details/78379312

//设置波特率
void SerialPort::set_brate()
{
    // name_arr为十进制， speed_arr为二进制
    int speed_arr[] = {B921600, B460800, B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
                       B921600, B460800, B115200, B38400, B19200, B9600, B4800, B2400, B1200, B300,
                      };
    int name_arr[] = {921600, 460800, 115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
                      921600, 460800, 115200, 38400, 19200, 9600, 4800, 2400, 1200,  300,
                     };
    unsigned int i;
    int status;
    struct termios Opt;     //用于存储获得的终端参数信息
    tcgetattr(fd, &Opt);
    for (i = 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
    {
        if (boudrate == name_arr[i])
        {
            tcflush(fd, TCIOFLUSH);                 //清空缓冲区的内容
            cfsetispeed(&Opt, speed_arr[i]);        //设置接受和发送的波特率
            cfsetospeed(&Opt, speed_arr[i]);
            status = tcsetattr(fd, TCSANOW, &Opt);  //使设置立即生效

            if (status != 0)
            {
                return;
            }

            tcflush(fd, TCIOFLUSH);

        }
    }
}

//设置串口数据位，停止位和效验位
int SerialPort::set_bit()
{
    // 获取串口相关参数
    struct termios termios_p;
    tcgetattr(fd, &termios_p);
    // CLOACL：保证程序不会占用串口
    // CREAD：使得能够从串口中读取输入数据
    termios_p.c_cflag |= (CLOCAL | CREAD);

    // 设置数据位
    termios_p.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 7:
        termios_p.c_cflag |= CS7;
        break;

    case 8:
        termios_p.c_cflag |= CS8;
        break;

    default:
        fprintf(stderr, "Unsupported data size\n");
        return (false);
    }

    // 设置奇偶校验位
    switch (parity)
    {
    case 'n':
    case 'N':
        termios_p.c_cflag &= ~PARENB;       //PARENB:启用奇偶校验码的生成和检测功能。
        termios_p.c_iflag &= ~INPCK;        //INPCK：对接收到的字符执行奇偶校检。
        break;

    case 'o':
    case 'O':
        termios_p.c_cflag |= (PARODD | PARENB); /* 设置为奇效验*/
        termios_p.c_iflag |= INPCK;             /* Disnable parity checking */
        break;

    case 'e':
    case 'E':
        termios_p.c_cflag |= PARENB;        /* Enable parity */
        termios_p.c_cflag &= ~PARODD;       /* 转换为偶效验*/
        termios_p.c_iflag |= INPCK;         /* Disnable parity checking */
        break;

    case 'S':
    case 's':  /*as no parity*/
        termios_p.c_cflag &= ~PARENB;
        termios_p.c_cflag &= ~CSTOPB;
        break;

    default:
        fprintf(stderr, "Unsupported parity\n");
        return (false);

    }

    // 设置停止位
    switch (stopbits)
    {
    case 1:
        termios_p.c_cflag &= ~CSTOPB;
        break;

    case 2:
        termios_p.c_cflag |= CSTOPB;
        break;

    default:
        fprintf(stderr, "Unsupported stop bits\n");
        return (false);

    }
    if (parity != 'n')
    {
        termios_p.c_iflag |= INPCK;
    }

    tcflush(fd, TCIFLUSH);                                  //清除输入缓存区
    termios_p.c_cc[VTIME] = 1;                              //设置超时
    termios_p.c_cc[VMIN] = 0;                 //最小接收字符
    termios_p.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);   //Input原始输入
    // 写死了，不管前面写了什么，都一定是无奇偶校验
    termios_p.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    termios_p.c_iflag &= ~(ICRNL | IGNCR);
    termios_p.c_oflag &= ~OPOST;                            //Output禁用输出处理

    if (tcsetattr(fd, TCSANOW, &termios_p) != 0)
    {
        return (false);
    }

    return (true);
}

bool SerialPort::init()
{
    fd = open("/dev/ttyUSB", O_RDWR | O_NOCTTY | O_NDELAY);   //非阻塞
    boudrate = 460800;
    databits = 8;
    stopbits = 1;
    parity = 'N';


    if (fd == -1)
    {
        return false;
    }

    //  设置波特率
    set_brate();

    //  设置数据位
    if (set_bit() == false)
    {
        exit(0);
    }

    printf("init successed\n");
    return true;
}

void SerialPort::close_port()
{
    close(fd);
}

bool SerialPort::recieve(CarData& cardata)
{
    unsigned int bytes;
    uint8_t header[1];
    memset(rdata, 0, 4096*sizeof(uint8_t));
    // 剔除残余字节
    double t1 = cv::getTickCount();
    // 相当于阻塞
    while(1)
    {
        ioctl(fd, FIONREAD, &bytes);
        // std::cout << bytes << std::endl;
        if( bytes >= 1)
        {
            // std::cout << bytes << std::endl;
            bytes = read(fd, header, 1);
            if(header[0] == 0xA5)
            {
                break;
            }
        }
        double t2 = cv::getTickCount();
        double delta_time = (t2 - t1) / cv::getTickFrequency() * 1000;
        // 超时时间
        if(delta_time > 10)
        {
            return false;
        }
    }


    memcpy(rdata, header, 1);  //rdata[0] = 0xA5
    // 以后每次只需读数据包长度的倍数就不会出现断帧的情况
    // 轮询：查看缓冲区有多少个字节：
    ioctl(fd, FIONREAD, &bytes);


    int multiple = (bytes + 1) / sizeof(CarData);   //倍数:几个数据包
    // std::cout << &rdata[1] << " " << rdata << std::endl;
    // 我们并不读取缓冲区所有字节，因为清空缓冲区会导致断帧的现象
    bytes = read(fd, &rdata[1], sizeof(CarData) * multiple - 1);
     if(Verify_CRC8_Check_Sum(&rdata[(multiple - 1) *sizeof(CarData)], 3) && Verify_CRC16_Check_Sum(&rdata[(multiple - 1) *sizeof(CarData) ],sizeof(CarData)))
    {
        memcpy(&cardata, &rdata[(multiple - 1) *sizeof(CarData)], sizeof(CarData));
        return true;
    }
    else
    {
        std::cout << "crc check failed!!! "  <<std::endl;
        return false;
    }

}
bool SerialPort::recieve2(CarData& cardata)
{
    int bytes;
    int result = ioctl(fd, FIONREAD, &bytes);
    if (result == -1)
    {
        #ifdef OPEN_COUT
            std::cout<<"result: "<<"-1"<< std::endl;
            printf("ioctl: %s\n", strerror(errno));
        #endif
        return false;
    }
    if (bytes == 0)
    {
        return false;
    }

    bytes = read(fd, rdata, bytes);
    int FirstIndex = -1;
    int flag = 0;
    for(int i = 0; i < bytes; i++)
    {
        if( rdata[i] == 0xA5 && FirstIndex == -1&& Verify_CRC8_Check_Sum(&rdata[i], 3) && Verify_CRC16_Check_Sum(&rdata[i],sizeof(CarData) ))
        {
            FirstIndex = i;
            flag = 1;
        }
        if(flag)
        {
            break;
        }
    }
    
    int max_mul = (bytes - FirstIndex) / sizeof(CarData);
    if(FirstIndex != -1)
    {
        if(rdata[FirstIndex + (max_mul - 1)* sizeof(CarData)] == 0xA5  && Verify_CRC8_Check_Sum(&rdata[FirstIndex + (max_mul - 1)* sizeof(CarData)], 3) )
        {
            memcpy(&cardata, &rdata[FirstIndex + (max_mul - 1) *sizeof(CarData)], sizeof(CarData));
            return true;
        }
        else
        {
            std::cout << "crc check failed!!! "  <<std::endl;
            return false;
        }
    }
    else
    {
         std::cout << " can't find header!!!" << std::endl;
        return false;
    }
}

bool SerialPort::check_data(VisionData& vd)
{
    
    return true;
}

bool SerialPort::check_data(const CarData& cardata)
{
    
    return true;
}

void SerialPort::send(VisionData& vd)
{
    if(check_data(vd))
    {
        memcpy(Tdata, &vd, sizeof(VisionData));
        Append_CRC8_Check_Sum(Tdata, 3);
        Append_CRC16_Check_Sum(Tdata, sizeof(VisionData));
        result = write(fd, Tdata, sizeof(VisionData));
        if(result == -1)
        {
            close_port();
            init();
        }
    }
    else
    {
        memset(&vd, 0, sizeof(VisionData));
        Append_CRC8_Check_Sum(Tdata, 3);
        Append_CRC16_Check_Sum(Tdata, sizeof(VisionData));
        result = write(fd, Tdata, sizeof(VisionData));
    }

}

SerialPort::SerialPort()
{
    if(!init())
    {
        return;
    }

}
SerialPort::~SerialPort()
{
    close_port();
}
