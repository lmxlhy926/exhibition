//
// Created by 78472 on 2022/8/24.
//

#include "downBinaryFlowControl.h"
#include "downBinaryUtil.h"
#include <chrono>

downBinaryFlowControl* downBinaryFlowControl::instance;

void downBinaryFlowControl::push(string &command) {
    std::lock_guard<std::mutex> lg(mutex_);
    commandQueue.push(command);
    cv.notify_one();
}

string downBinaryFlowControl::fetchCommand() {
    std::lock_guard<std::mutex> lg(mutex_);
    if(!commandQueue.empty()){
        string command = commandQueue.front();
        commandQueue.pop();
        return command;
    }
    return std::string();
}

void downBinaryFlowControl::sendCommand() {
    while(true){
        if(commandQueue.empty()){
            std::unique_lock<std::mutex> l(mutex_);
            cv.wait(l, [this](){ return !commandQueue.empty(); });
        }

        string command = fetchCommand();
        if(!command.empty()){
            unsigned char buf[100]{};
            size_t size = DownBinaryUtil::binaryString2binary(command, buf, sizeof buf);
            DownBinaryUtil::serialSend(buf, static_cast<int>(size));
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
}


