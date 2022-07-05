//
// Created by 78472 on 2022/6/13.
//

#include "binary2JsonEvent.h"
#include <iostream>
#include <cstring>
#include <iomanip>

bool Binary2JsonEvent::binary2JsonEvent(unsigned char *binaryStream, int size) {
    if(size > 512)  return false;

    int index = 0;
    unsigned char buf[512];
    memset(buf, 0, 512);

    if(binaryStream[0] == 0x01 && binaryStream[ size- 1] == 0x03){
        for(int i = 1; i < size - 1; i++){
            if(binaryStream[i] == 0x02){
                buf[index++] = binaryStream[i + 1] & 0x0f;
                ++i;
            }else{
                buf[index++] = binaryStream[i];
            }
        }

        stringstream ss;
        for( int i = 0; i < index; i++){
            ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(buf[i]);
        }

        string binaryString = ss.str();
        printBinaryString(binaryString);
        postEvent(binaryString);

       return true;
    }

    return false;
}


void Binary2JsonEvent::printBinaryString(string &str) {
    stringstream ss;
    for(int i = 0; i < str.size() / 2; ++i){
        ss << str.substr(i * 2, 2);
        if(i < str.size() / 2 - 1)
            ss << " ";
    }
    std::cout << "==>serial receive: " << ss.str() << std::endl;
}

void Binary2JsonEvent::postEvent(string &statusString) {
    string hciType, subType, packageIndex;
    ReadBinaryString rs(statusString);
    rs.read2Byte().readByte(hciType).readByte(subType).readByte(packageIndex);


    if(hciType == "91" && subType == "88"){     //扫描结果上报


    }else if(hciType == "91" && subType == "B2"){   //节点配置完成


    }else if(hciType == "91" && subType == "82"){   //绑定成功


    }else if(hciType == "91" && subType == "81"){     //开关命令上报，灯亮度上报
        string opcode;
        rs.read2Byte().read2Byte().read2Byte(opcode);
        if(opcode == "8204"){
            rs.rollBack(6);
            std::cout << "==>LightOnOffStatus: " << LightOnOffStatus(rs.remainingString()).construct() << std::endl;

        }else if(opcode == "824E"){
            rs.rollBack(6);
            std::cout << "==>LightBrightStatus: " << LightBrightStatus(rs.remainingString()).construct() << std::endl;
        }
    }
}

