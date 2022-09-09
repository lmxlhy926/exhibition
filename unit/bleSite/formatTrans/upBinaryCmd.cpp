//
// Created by 78472 on 2022/6/13.
//

#include "upBinayCmd.h"
#include <iostream>
#include <cstring>
#include <iomanip>
#include "log/Logging.h"
#include "statusEvent.h"

using namespace muduo;

#include "statusEvent.h"

void tempPrint(const unsigned char *binaryStream, int size){
    stringstream ss;
    for (int i = 0; i < size; i++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(binaryStream[i]);
    }
    LOG_INFO << ss.str();
}


/*
 * 1. 处理转义数据
 * 2. 二进制格式转换为字符串形式
 */
static string binaryCmd2String(const unsigned char *binaryStream, int size) {
    if (size > 512) return string();

    int index = 0;
    unsigned char buf[512];
    memset(buf, 0, 512);

    //去掉包头标识符、包尾标识符，转义包内容
    if (binaryStream[0] == 0x01 && binaryStream[size - 1] == 0x03) {
        for (int i = 1; i < size - 1; i++) {
            if (binaryStream[i] == 0x02) {
                buf[index++] = binaryStream[i + 1] & 0x0f;
                ++i;
            } else {
                buf[index++] = binaryStream[i];
            }
        }

        stringstream ss;
        for (int i = 0; i < index; i++) {
            ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(buf[i]);
        }
        return ss.str();
    }

    return string();
}

void printBinaryString(string &str) {
    stringstream ss;
    for(int i = 0; i < str.size() / 2; ++i){
        ss << str.substr(i * 2, 2);
        if(i < str.size() / 2 - 1)
            ss << " ";
    }
    LOG_INFO << "==>serial receive: " << ss.str();
}

string UpBinaryCmd::packageString;
std::mutex UpBinaryCmd::mutex_;

bool UpBinaryCmd::bleReceiveFunc(unsigned char *binaryStream, int size) {
    std::lock_guard<std::mutex> lg(mutex_);
    string binaryString = binaryCmd2String(binaryStream, size);

    //组包
    string hciType, subType, packageIndex;
    ReadBinaryString rs(binaryString);
    rs.read2Byte().readByte(hciType).readByte(subType).readByte(packageIndex);

    if(packageIndex == "00"){   //整包
        PostStatusEvent(binaryString).operator()();

    }else if(packageIndex == "01"){
        packageString.clear();
        packageString.append(binaryString);

    }else if(packageIndex == "02"){
        packageString.append(rs.remainingString());

    }else if(packageIndex == "03"){
        packageString.append(rs.remainingString());
        PostStatusEvent(packageString).operator()();
        packageString.clear();
    }

    return true;
}

