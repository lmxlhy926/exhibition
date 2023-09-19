//
// Created by 78472 on 2022/9/8.
//

#include "telinkDongle.h"
#include <cstring>
#include <log/Logging.h>
#include <iomanip>
#include <sstream>
#include "sourceManage/bleConfig.h"
#include "upStatus/statusEvent.h"

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
        sendListStoreHandle(commandString);
    }
    sendConVar.notify_one();
    return true;
}

//向串口写入三火开关控制数据
bool TelinkDongle::write2Serial_tripleSwitch(std::string& commandString){
    {
        std::lock_guard<std::mutex> lg(sendMutex);
        sendList_tripleSwitch.push_back(commandString);
    }
    sendConVar.notify_one();
    return true;
}

//删除队列里的三火开关控制数据
void TelinkDongle::deleteTripleSwitchControlData(string address){
    if(address.empty()) return;
    std::lock_guard<std::mutex> lg(sendMutex);
    if(addressTryCountMap.find(address) != addressTryCountMap.end()){
        addressTryCountMap.erase(address);
    }
    for(auto pos = sendList_tripleSwitch.begin(); pos != sendList_tripleSwitch.end(); ++pos){
        string elemAddress = getCommandAddress(*pos);
        if(elemAddress == address){
            sendList_tripleSwitch.erase(pos);
            return;
        }
    }
}


void TelinkDongle::registerPkgMsgFunc(PackageMsgHandleFuncType fun) {
    packageMsgHandledFunc = std::move(fun);
}

bool TelinkDongle::startReceive() {
    if(!commonSerial.isOpened()){
        LOG_RED << commonSerial.getSerialName() << " is not opened....";
        return false;
    }

    //从发送队列读取数据，发送
    sendCommandThread = new std::thread([this]() {
        sendThreadFunc();
    });

    //从串口读取数据，拼接为独立包，加入接收队列
    receiveThread = new std::thread([this](){
        handleReceiveData();
    });

    //从接收队列读取子包，拼接为完整包，处理上报包数据
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
        string commandString;
        {
            std::unique_lock<std::mutex> ul(sendMutex);
            if(sendList.empty() && sendList_tripleSwitch.empty()){  //如果发送列表没有数据则等待
                sendConVar.wait(ul, [this](){
                    return !sendList.empty() || !sendList_tripleSwitch.empty() || !commonSerial.isOpened();
                });
            }

            if(!commonSerial.isOpened()){
                LOG_RED << "quit sendThreadFunc, serial closed....";
                return;
            }

            //从队列取得命令字符串
            commandString = getCommandString();
        }

        if(commonSerial.isOpened()){
            std::vector<uint8_t> commandVec = commandString2BinaryByte(commandString);
            if(commonSerial.write2Serial(commandVec.data(), static_cast<int>(commandVec.size()))){
                LOG_HLIGHT << "==>sendCmd<true>: " << binary2SendString(commandVec);
            }else{
                LOG_HLIGHT << "==>sendCmd<false>: " << binary2SendString(commandVec);
            }
            this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
}


void TelinkDongle::sendListStoreHandle(std::string& commandString){
    //如果是组控指令，删除之前有的相同指令
//    ReadBinaryString rs(commandString);
//    string groupIdExtract, opCodeExtract;
//    rs.readBytes(8).read2Byte(groupIdExtract).read2Byte(opCodeExtract);
//    if (opCodeExtract == "825F") {      //删除重复组控指令
//        for(auto pos = sendList.begin(); pos != sendList.end();){
//            ReadBinaryString elemRs(*pos);
//            string elemGroup, elemOpcode;
//            elemRs.readBytes(8).read2Byte(elemGroup).read2Byte(elemOpcode);
//            if(elemOpcode == "825F" && elemGroup == groupIdExtract){
//                pos = sendList.erase(pos);
//            }else{
//                pos++;
//            }
//        }
//    }
//
//    if(groupIdExtract == "01C0" || groupIdExtract == "04C0"){
//        sendList.push_front(commandString);
//    }else{
//        sendList.push_back(commandString);
//    }

    sendList.push_back(commandString);
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
                    LOG_YELLOW << "<<==packageString: " << packageString;
                    recvQueue.push(packageString);
//                    LOG_INFO << "remainSize: " << recvQueue.size();
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


//提取命令中的地址
string TelinkDongle::getCommandAddress(string commandString){
    string address = commandString.substr(8 * 2, 4);
    return address;
}

//记录三火开关数据
string TelinkDongle::recordTripleSwitch(string commandString){
    commandString = commandString.substr(0, commandString.size() - 4);
    string address = getCommandAddress(commandString);
    auto pos = addressTryCountMap.find(address);
    if( pos != addressTryCountMap.end()){
        pos->second++;
        stringstream ss;
        ss << std::setw(2) << std::setfill('0') << pos->second;
        commandString.append(ss.str());
        if(pos->second >= 6){
            addressTryCountMap.erase(pos);
            sendList_tripleSwitch.pop_front();
        }
    }else{
        addressTryCountMap.insert(std::make_pair(address, 1));
        stringstream ss;
        ss << std::setw(2) << std::setfill('0') << 1;
        commandString.append(ss.str());
    }
    common = false;
    triple = true;
    return commandString;
}

string TelinkDongle::getCommandString(){
    string commandString;
    if(!sendList_tripleSwitch.empty() && sendList.empty()){ 
        commandString = sendList_tripleSwitch.front();
        commandString = recordTripleSwitch(commandString);

    }else if(!sendList.empty() && sendList_tripleSwitch.empty()){
        commandString = sendList.front();
        sendList.pop_front();
        common = true;
        triple = false;

    }else{
        if(!common){
            commandString = sendList.front();
            sendList.pop_front();
            common = true;
            triple = false;

        }else if(!triple){
            commandString = sendList_tripleSwitch.front();
            commandString = recordTripleSwitch(commandString);
        }
    } 
    return commandString;  
}
