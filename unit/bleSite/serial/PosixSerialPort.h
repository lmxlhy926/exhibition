//
// Created by WJG on 2022-6-1.
//

#ifndef BLE_LIGHT_SITE_POSIXSERIALPORT_H
#define BLE_LIGHT_SITE_POSIXSERIALPORT_H

#include "BaseSerialPort.h"

class PosixSerialPort : public BaseSerialPort {

public:
    PosixSerialPort();
    ~PosixSerialPort();

    /*
     * 1. 打开串口文件
     * 2. 设置串口通信参数
     */
    bool initSerial(std::string serial_name, SerialParamStruct aStruct) override;

    /*
     * 向串口文件中写数据
     * 一次写操作最多执行三次，最多写128字节
     */
    bool writeSerialData(unsigned char *buff, int len) override;


protected:
    /*
     * 从文件中循环读取数据
     * 每次读操作间隔100ms
     */
    void readSerialData() override;

private:
    const static int8_t ival_comm_write_try_times_ = 3;     //最大写入次数
    int fd_serial;                                          //串口文件描述符
    bool setParity(int databits,int stopbits,int parity);   //设置串口数据位、停止位、奇偶位
    void setSpeed(int speed);                               //设置波特率
};

#endif //BLE_LIGHT_SITE_POSIXSERIALPORT_H
