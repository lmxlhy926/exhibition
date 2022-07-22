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

    bool initSerial(std::string serial_name, SerialParamStruct aStruct) override;
    bool writeSerialData(unsigned char *buff, int len) override;


protected:
    void readSerialData() override;

private:
    const static int8_t ival_comm_write_try_times_ = 3;     //最大写入次数
    int fd_serial;                                          //串口文件描述符
    bool setParity(int databits,int stopbits,int parity);   //设置串口数据位、停止位、奇偶位
    void setSpeed(int speed);                               //设置波特率
    void procSerialData();
};

#endif //BLE_LIGHT_SITE_POSIXSERIALPORT_H
