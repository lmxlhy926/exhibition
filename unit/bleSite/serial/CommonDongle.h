//
// Created by WJG on 2022-5-31.
//

#ifndef BLE_LIGHT_SITE_COMMONDONGLE_H
#define BLE_LIGHT_SITE_COMMONDONGLE_H

#include "BaseSerialPort.h"

#define DATA_BUFF_SIZE 1024

//定义接收回调函数原型
typedef bool (*RECV_DATA_PROC)(unsigned char *data, int len);

/*
 * 初始化串口参数
 * 开启/关闭串口读功能
 * 串口写数据
 * 设置串口接收数据处理回调函数
 */
class CommonDongle{
public:
    explicit CommonDongle(std::string port_name);

    virtual ~CommonDongle();

    //初始化串口参数，由具体的继承类来设置
    virtual bool initDongle();

    //开启读取串口数据，必须事先设置回调函数
    bool startDongle();

    //关闭读取串口数据
    bool stopDongle();

    //向串口发送数据，最大不超过DATA_BUFF_SIZE个字节
    bool sendData(unsigned char *data, int len);

    //设置串口接收数据的回调处理函数
    bool regRecvDataProc(RECV_DATA_PROC proc_fun);

protected:
    std::string serial_name;                    //指定的串口名
    BaseSerialPort *pSerial = nullptr;          //真正的串口操作对象
    RECV_DATA_PROC recv_data_proc = nullptr;    //处理接收数据的回调函数
};

#endif //BLE_LIGHT_SITE_COMMONDONGLE_H
