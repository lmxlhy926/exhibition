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
#include <list>

#define MaxReadOneShot (1024)

//包消息处理
using PackageMsgHandleFuncType = std::function<bool(std::string&)>;

class TelinkDongle {
private:
    static TelinkDongle* instance;
    CommonSerial commonSerial;

    std::queue<string> recvQueue;     //包队列
    std::mutex recvMutex;
    std::condition_variable recvConVar;

    std::list<string> sendList;     //发送队列
    std::mutex sendMutex;
    std::condition_variable sendConVar;

    std::vector<uint8_t> packageFrame;  //存储一个完整的未转义的包数据
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

    explicit TelinkDongle(std::string& serial_name);

public:
    static TelinkDongle* getInstance(std::string& serial_name){
        if(instance == nullptr){
            instance = new TelinkDongle(serial_name);
        }
        return instance;
    }

    ~TelinkDongle();

    //按照指定的参数打开串口
    bool openSerial(SerialParamStruct aStruct);

    //关闭串口
    void closeSerial();

    //向串口写数据
    bool write2Seria(std::string& commandString);

    //注册包消息处理函数
    void registerPkgMsgFunc(PackageMsgHandleFuncType fun);

    //开始读取并处理从串口读取到的数据
    bool startReceive();

private:
    std::vector<uint8_t> commandString2BinaryByte(string& commandString);   //字符串转换为二进制数据

    string binary2SendString(std::vector<uint8_t>& sendVec);    //打印待发送的数据

    void sendThreadFunc();  //发送数据指令线程例程

    void queueHandle();

    void handleReceiveData();    //从串口读取数据，截取到数据包后，存入队列

    void joinPackage(uint8_t *data, ssize_t length);  //拼接包数据、转义、加入队列

    string packageEscape(std::vector<uint8_t>& originFrame) const;  //转义包数据后，转换为字符串

    void packageHandel();   //从队列中读取包并处理
};


#endif //EXHIBITION_TELINKDONGLE_H
