//
// Created by 78472 on 2022/9/8.
//

#include "telinkDongle.h"
#include <cstring>
#include <log/Logging.h>


TelinkDongle::TelinkDongle(std::string& serial_name)
    : commonSerial(serial_name){}

/**
 * 关闭串口，退出读取线程
 */
TelinkDongle::~TelinkDongle() {
    closeSerial();
    if(read_thread_!= nullptr && read_thread_->joinable())
        read_thread_->join();   //阻塞回收线程
    delete read_thread_;        //线程析构
    read_thread_ = nullptr;
}

bool TelinkDongle::openSerial(SerialParamStruct aStruct){
    return commonSerial.openSerial(aStruct);
}

void TelinkDongle::closeSerial() {
    commonSerial.closeSerial();
}

bool TelinkDongle::write2Seria(std::vector<uint8_t>& data) {
    {
        std::lock_guard<std::mutex> lg(sendMutex);
        sendQueue.push(data);
    }
    sendConVar.notify_one();
}

void TelinkDongle::registerPkgMsgFunc(PackageMsgHandleFuncType fun) {
    packageMsgHandledFunc = std::move(fun);
}

bool TelinkDongle::startReceive() {
    if(!commonSerial.isOpened()){
        LOG_RED << commonSerial.getSerialName() << " is not opened....";
        return false;
    }

    //创建线程，读取数据
    read_thread_ = new std::thread([this]() {
        readFromSerialAndHandle();
    });

    return true;
}

void TelinkDongle::sendThreadFunc(){
    while(true){
        std::unique_lock<std::mutex> ul(sendMutex);
        if(sendQueue.empty()){
            sendConVar.wait(ul, [this](){
                return !sendQueue.empty();
            });
        }

        std::vector<uint8_t> message = sendQueue.front();
        sendQueue.pop();
        if(commonSerial.isOpened()){
            commonSerial.write2Serial(message.data(), static_cast<int>(message.size()));
            this_thread::sleep_for(std::chrono::milliseconds(800));
        }else{
            break;
        }
    }
}


void TelinkDongle::handleReceiveData() {
    fd_set readSet, allset;
    FD_ZERO(&allset);
    FD_SET(commonSerial.getSerialFd(), &allset);

    while (true)
    {
        readSet = allset;
        if(commonSerial.isOpened()){
            int nReady = select(commonSerial.getSerialFd() + 1, &readSet, nullptr, nullptr, nullptr);
            if(nReady > 0){
                uint8_t buffer[MaxReadOneShot]{};
                ssize_t nRead = commonSerial.readFromSerial(buffer, MaxReadOneShot);
                if(nRead > 0){
                    joinPackage(buffer, nRead);
                }
            }else{
                break;
            }
        }else{
            break;
        }
    }
}

void TelinkDongle::joinPackage(uint8_t *data, ssize_t length){
    for(ssize_t i = 0; i < length; ++i){
        if(parseState == HEAD){
            if(data[i] == PackageStartIndex){
                packageFrame.push_back(data[i]);
                parseState = DATA;
            }else{
                continue;
            }

        }else if(parseState == DATA){
            packageFrame.push_back(data[i]);
            if(data[i] == PackageEndIndex){
                {
                    std::lock_guard<std::mutex> lg(recvMutex);
                    recvQueue.push(packageFrame);
                }
                recvConVar.notify_one();
                packageFrame.clear();
                parseState = HEAD;
            }
        }
    }
}

void TelinkDongle::packageHandel(){
    while(true){
        std::unique_lock<std::mutex> ul(recvMutex);
        if(recvQueue.empty()){
            recvConVar.wait(ul, [this](){
                return !recvQueue.empty();
            });
        }

        std::vector<uint8_t> packageMessage = recvQueue.front();
        recvQueue.pop();
        packageMsgHandledFunc(packageMessage);
    }
}

