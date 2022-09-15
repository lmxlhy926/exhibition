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

class PosixSerial {
private:
    string serialName;                                      //串口名称
    int fd_serial;                                          //串口文件描述符
    std::atomic<bool> isSerialOpened{false};             //串口是否打开
    std::mutex readMutex;                                   //保护并发读
    std::mutex writeMutex;                                  //保护并发写
    const static int8_t ival_comm_write_try_times_ = 3;     //最大写入次数

public:
    explicit PosixSerial(std::string& serial_name, SerialParamStruct aStruct);

    ~PosixSerial();

    /*
     * 向串口文件中写数据
     * 一次写操作最多执行三次，最多写128字节
     */
    bool writeSerialData(unsigned char *buff, int writeLen);

    //读取数据到指定buffer中
    ssize_t readSerialData(unsigned char *receiveBuff, int readLen);

    //关闭串口文件, 设置isSerialOpened为false.
    void closeSerial();

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
    bool initSerial(std::string& serial_name, SerialParamStruct aStruct);   //打开串口文件，设置通信参数

    bool setParity(int databits,int stopbits,int parity);   //设置串口数据位、停止位、奇偶位

    bool setSpeed(int speed) const;   //设置波特率
};

#endif //EXHIBITION_POSIXSERIAL_H
