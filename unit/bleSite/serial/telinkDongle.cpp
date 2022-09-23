//
// Created by 78472 on 2022/9/8.
//

#include "telinkDongle.h"
#include <cstring>
#include <log/Logging.h>
#include <iomanip>
#include <sstream>

TelinkDongle* TelinkDongle::instance = nullptr;

TelinkDongle::TelinkDongle(std::string& serial_name)
    : commonSerial(serial_name){}

/**
 * 关闭串口，退出读取线程
 */
TelinkDongle::~TelinkDongle() {
    closeSerial();
}

bool TelinkDongle::openSerial(SerialParamStruct aStruct){
    return commonSerial.openSerial(aStruct);
}

void TelinkDongle::closeSerial() {
    commonSerial.closeSerial();
}

bool TelinkDongle::write2Seria(std::string& commandString) {
    {
        std::lock_guard<std::mutex> lg(sendMutex);
        sendQueue.push(commandString);
    }
    sendConVar.notify_one();
    return true;
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
    sendCommandThread = new std::thread([this]() {
        sendThreadFunc();
    });

    receiveThread = new std::thread([this](){
        handleReceiveData();
    });

    packageHandleThread = new std::thread([this](){
        packageHandel();
    });

    return true;
}

std::vector<uint8_t> TelinkDongle::commandString2BinaryByte(string& commandString){
    std::vector<uint8_t> commandVec;
    for(int i = 0; i < commandString.size() / 2; i++) {
        uint8_t hexByte;
        try {
            hexByte = std::stoi(commandString.substr(i * 2, 2), nullptr, 16);
        } catch (std::exception &e) {
            hexByte = 0;
        }
        commandVec.push_back(hexByte);
    }
    return commandVec;
}

string TelinkDongle::binary2SendString(std::vector<uint8_t>& sendVec) {
    stringstream ss;
    for(int i = 0; i < sendVec.size(); ++i){
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<unsigned int>(sendVec[i]);
        if(i != sendVec.size() -1){
            ss << " ";
        }
    }
    return ss.str();
}

void TelinkDongle::sendThreadFunc(){
    while(true){
        std::unique_lock<std::mutex> ul(sendMutex);
        if(sendQueue.empty()){  //如果发送列表没有数据则等待
            sendConVar.wait(ul, [this](){
                return !sendQueue.empty() || !commonSerial.isOpened();
            });
        }

        if(!commonSerial.isOpened()){
            LOG_RED << "quit sendThreadFunc, serial closed....";
            return;
        }

        string commandString = sendQueue.front();
        sendQueue.pop();
        if(commonSerial.isOpened()){
            std::vector<uint8_t> commandVec = commandString2BinaryByte(commandString);
            if(commonSerial.write2Serial(commandVec.data(), static_cast<int>(commandVec.size()))){
                LOG_HLIGHT << "==>sendCmd<true>: " << binary2SendString(commandVec);
            }else{
                LOG_HLIGHT << "==>sendCmd<false>: " << binary2SendString(commandVec);
            }
            this_thread::sleep_for(std::chrono::milliseconds(1000));

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
            LOG_INFO << "WAIT.....";
            int nReady = select(commonSerial.getSerialFd() + 1, &readSet, nullptr, nullptr, nullptr);
            LOG_INFO << "nReady: " << nReady;
            if(nReady > 0){
                uint8_t buffer[MaxReadOneShot]{};
                ssize_t nRead = commonSerial.readFromSerial(buffer, MaxReadOneShot);
                LOG_INFO << "nRead: " << nRead;
                if(nRead > 0){
                    joinPackage(buffer, nRead);
                }
            }
        }else{
            LOG_RED << "QUIT handleReceiveData....";
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

        }else{
            packageFrame.push_back(data[i]);
            if(data[i] == PackageEndIndex){
                string packageString = packageEscape(packageFrame); //转义包数据
                packageFrame.clear();
                parseState = HEAD;
                {
                    std::lock_guard<std::mutex> lg(recvMutex);
                    LOG_YELLOW << "===>packageString: " << packageString;
                    recvQueue.push(packageString);
                    LOG_INFO << "remainSize: " << recvQueue.size();
                }
                recvConVar.notify_one();
            }
        }
    }
}

string TelinkDongle::packageEscape(std::vector<uint8_t>& originFrame) const{
    std::vector<uint8_t> escVec;
    for(ssize_t i = 0; i < originFrame.size(); ++i){
        if(originFrame[i] == PackageStartIndex || originFrame[i] == PackageEndIndex){
            continue;

        }else if(originFrame[i] == 0x02){
            escVec.push_back(originFrame[++i] & 0x0f);

        }else{
            escVec.push_back(originFrame[i]);
        }
    }

    stringstream ss;
    for(auto& elem : escVec){
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<unsigned int>(elem);
    }
    return ss.str();
}

void TelinkDongle::packageHandel(){
    while(true){
        std::unique_lock<std::mutex> ul(recvMutex);
        if(recvQueue.empty()){  //队列中没有包数据，则等待
            recvConVar.wait(ul, [this](){
                return !recvQueue.empty() || !commonSerial.isOpened();
            });
        }

        if(!commonSerial.isOpened())
            return;

        string packageString = recvQueue.front();
        recvQueue.pop();
        packageMsgHandledFunc(packageString);
    }
}

