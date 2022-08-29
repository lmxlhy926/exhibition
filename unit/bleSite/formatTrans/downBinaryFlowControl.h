//
// Created by 78472 on 2022/8/24.
//

#ifndef EXHIBITION_DOWNBINARYFLOWCONTROL_H
#define EXHIBITION_DOWNBINARYFLOWCONTROL_H

#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std;

class downBinaryFlowControl {
private:
    queue<string>            commandQueue;
    std::mutex               mutex_;
    std::condition_variable  cv;
    std::thread*             runningThread;

    downBinaryFlowControl(){
        runningThread = new thread([this](){
            sendCommand();
        });
    }

    static downBinaryFlowControl* instance;

public:
    static downBinaryFlowControl * getInstance(){
        if(!instance){
            instance = new downBinaryFlowControl();
        }
        return instance;
    }

    //向队列中添加命令
    void push(string& command);

    //从队列中获取命令
    string fetchCommand();

    //在独立线程中发送命令
    void sendCommand();
};


#endif //EXHIBITION_DOWNBINARYFLOWCONTROL_H
