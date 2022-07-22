//
// Created by WJG on 2022-5-31.
//

#ifndef BLE_LIGHT_SITE_COMMONDONGLE_H
#define BLE_LIGHT_SITE_COMMONDONGLE_H

#include "BaseSerialPort.h"

#define DATA_BUFF_SIZE 1024

//定义回调函数类型
typedef bool (*RECV_DATA_PROC)(unsigned char *data, int len);

class CommonDongle{
public:
    explicit CommonDongle(std::string port_name);
    virtual ~CommonDongle();

    //初始化串口参数，由具体的继承类来设置
    virtual bool initDongle();
    //打开串口，必须事先设置回调函数
    bool startDongle();
    //关闭串口
    bool stopDongle();
    //向串口发送数据
    bool sendData(unsigned char *data, int len);
    //设置串口回调函数
    bool regRecvDataProc(RECV_DATA_PROC proc_fun);

protected:
    std::string serial_name;
    BaseSerialPort *pSerial = nullptr;          //真正的串口操作对象
    RECV_DATA_PROC recv_data_proc = nullptr;
};

#endif //BLE_LIGHT_SITE_COMMONDONGLE_H
