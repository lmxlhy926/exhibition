//
// Created by 78472 on 2022/6/7.
//

#include "downBinaryUtil.h"
#include "log/Logging.h"
#include <sstream>
#include <iomanip>
#include <mutex>

std::mutex DownBinaryUtil::sendMutex;

size_t DownBinaryUtil::binaryString2binary(string &binaryString, unsigned char *buf, size_t size) {
    LOG_INFO<< binaryString;
    BinaryBuf binaryBuf(buf, size);
    for(int i = 0; i < binaryString.size() / 2; i++){
        string charString = binaryString.substr(i * 2, 2);
        binaryBuf.append(charString);
    }
    return binaryBuf.size();
}

bool DownBinaryUtil::serialSend(unsigned char *buf, int size) {
    std::lock_guard<std::mutex> lg(sendMutex);
    shared_ptr<TelinkDongle> serial = bleConfig::getInstance()->getSerial();
    if(serial != nullptr){
        if(serial->write2Seria(buf, size)){
            printSendBinary(buf, size);
            return true;
        }
    }
    return false;
}

void DownBinaryUtil::printSendBinary(unsigned char *buf, int size) {
    stringstream ss;
    for(int i = 0; i < size; i++){
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(buf[i]);
        if(i < size -1)
            ss << " ";
    }
    LOG_HLIGHT << "==>sendCmd: " << ss.str();
}


