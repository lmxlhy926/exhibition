
#ifndef NIGHTLIGHT_SENDBUFFER_H
#define NIGHTLIGHT_SENDBUFFER_H

#include <string>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "qlibc/QData.h"
using namespace std;

class sendBuffer{
private:
    static sendBuffer* Instance;
    std::mutex Mutex;
    std::condition_variable cv;
    std::queue<Json::Value> queue;
    std::thread* threadPtr = nullptr;

    sendBuffer(){
        threadPtr = new thread([this](){
            sendCommand();
        });
    }
public:
    //获取全局对象；
    static sendBuffer* getInstance(){
        if(Instance == nullptr){
            Instance = new sendBuffer();
        }
        return Instance;
    }

    //压入命令
    void enque(const Json::Value& command);

private:
    //取得命令，发送
    void sendCommand();
};


#endif




































