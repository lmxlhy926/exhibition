
#ifndef NIGHTLIGHT_SENDBUFFER_H
#define NIGHTLIGHT_SENDBUFFER_H

#include <queue>
#include <string>
#include <mutex>
#include <thread>
#include <condition_variable>
using namespace std;

class sendBuffer{
private:
    static sendBuffer* Instance;
    std::mutex Mutex;
    std::condition_variable cv;
    std::queue<string> queue;
    std::thread* threadPtr;

    sendBuffer(){
        threadPtr = new thread([this](){
            sendCommand();
        });
    }
public:
    //获取全局对象；
    static sendBuffer* getInstance();

    //压入命令
    void enque(const string& command);

private:
    //取得命令，发送
    void sendCommand();
};


#endif




































