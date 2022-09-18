//
// Created by 78472 on 2022/9/8.
//

#ifndef EXHIBITION_POSIXSERIAL_H
#define EXHIBITION_POSIXSERIAL_H

#include <cstdint>
#include <string>
#include <atomic>
#include <mutex>

using namespace std;

//设置串口参数结构体
typedef struct serial_param{
    int databits;   //数据位
    int stopbits;   //停止位
    int parity;     //奇偶位
    int baudrate;   //波特率
}SerialParamStruct;


/**
 *  创建对象时自动打开对应的串口文件， 先调用isOpened()判断串口是否打开
 *  进行串口的数据读写，如果读写过程中发生错误，会关闭串口文件
 */

class CommonSerial {
private:
    string serialName;                                      //串口名称
    int fd_serial;                                          //串口文件描述符
    std::atomic<bool> isSerialOpened{false};             //串口是否打开

public:
    explicit CommonSerial(std::string& serial_name);

    ~CommonSerial();

    //用指定参数打开串口
    bool openSerial(SerialParamStruct aStruct);

    //关闭串口文件, 设置isSerialOpened为false.
    bool closeSerial();

    //像串口写数据
    bool write2Serial(const void *buff, int writeLen);

    //从串口读数据
    ssize_t readFromSerial(void *receiveBuff, int readLen);

    //串口是否打开
    bool isOpened(){
        return isSerialOpened.load();
    }

    //获取串口文件对应的描述符
    int getSerialFd () const{
        return fd_serial;
    }

    //获取串口名字
    string getSerialName() const{
        return serialName;
    }

private:
    bool setProperty(int databits,int stopbits,int parity, int speed);   //设置串口数据位、停止位、奇偶位

    bool setSpeed(int speed) const;   //设置波特率

    bool setUartProperty();
};

#endif //EXHIBITION_POSIXSERIAL_H
