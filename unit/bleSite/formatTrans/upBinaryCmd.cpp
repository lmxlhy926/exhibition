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

/*
 * 1. 处理转义数据
 * 2. 二进制格式转换为字符串形式
 */
static string binaryCmd2String(const unsigned char *binaryStream, int size) {
    if (size > 512) return string();

    int index = 0;
    unsigned char buf[512];
    memset(buf, 0, 512);

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


bool UpBinaryCmd::bleReceiveFunc(unsigned char *binaryStream, int size) {
    string binaryString = binaryCmd2String(binaryStream, size);
    PostStatusEvent(binaryString).operator()();
    return true;
}

