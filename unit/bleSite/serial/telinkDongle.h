//
// Created by 78472 on 2022/9/8.
//

#ifndef EXHIBITION_TELINKDONGLE_H
#define EXHIBITION_TELINKDONGLE_H

#include "posixSerial.h"
#include <thread>
#include <functional>
#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>

#define MaxReadOneShot (1024)

//包消息处理
using PackageMsgHandleFuncType = std::function<bool(std::vector<uint8_t>&)>;

class TelinkDongle {
private:
    CommonSerial commonSerial;

    std::queue<std::vector<uint8_t>> recvQueue;
    std::mutex recvMutex;
    std::condition_variable recvConVar;

    std::queue<std::vector<uint8_t>> sendQueue;
    std::mutex sendMutex;
    std::condition_variable sendConVar;

    std::vector<uint8_t> packageFrame;
    PackageMsgHandleFuncType packageMsgHandledFunc = nullptr;   //处理读取数据的回调函数

    enum state{
        HEAD,
        DATA,
    };
    enum state parseState = HEAD;
    uint8_t PackageStartIndex = 0x01;    //包起始字符
    uint8_t PackageEndIndex = 0x03;     //包终止字符

    std::thread *sendCommandThread = nullptr;         //从队列取消息发送至串口
    std::thread *receiveThread = nullptr;             //从串口读消息，拼接为包，加入队列
    std::thread *packageHandleThread = nullptr;       //从队列读取包数据进行处理

public:
    explicit TelinkDongle(std::string& serial_name);

    ~TelinkDongle();

    //按照指定的参数打开串口
    bool openSerial(SerialParamStruct aStruct);

    //关闭串口
    void closeSerial();

    //向串口写数据
    bool write2Seria(std::vector<uint8_t>& data);

    //注册包消息处理函数
    void registerPkgMsgFunc(PackageMsgHandleFuncType fun);

    //开始读取并处理从串口读取到的数据
    bool startReceive();

private:
    void sendThreadFunc();  //发送数据指令线程例程

    void handleReceiveData();    //从串口读取数据并处理

    void joinPackage(uint8_t *data, ssize_t length);  //拼接为数据包后，加入队列

    void packageHandel();   //从队列中读取包并处理
};


#endif //EXHIBITION_TELINKDONGLE_H
