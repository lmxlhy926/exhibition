//
// Created by 78472 on 2022/6/13.
//

#include "recvPackageParse.h"
#include <iostream>
#include <cstring>
#include <iomanip>
#include "log/Logging.h"
#include "statusEvent.h"

using namespace muduo;

#include "statusEvent.h"

void printBinaryString(string &str) {
    stringstream ss;
    for(int i = 0; i < str.size() / 2; ++i){
        ss << str.substr(i * 2, 2);
        if(i < str.size() / 2 - 1)
            ss << " ";
    }
    LOG_INFO << "==>serial receive: " << ss.str();
}

string RecvPackageParse::packageString;
std::mutex RecvPackageParse::mutex_;
bool RecvPackageParse::enable = true;

bool RecvPackageParse::handlePackageString(string& subPackageString) {
    if(enable){
        std::lock_guard<std::mutex> lg(mutex_);
        string hciType, subType, packageIndex;
        ReadBinaryString rs(subPackageString);
        rs.read2Byte().readByte(hciType).readByte(subType).readByte(packageIndex);

        if(packageIndex == "00"){   //整包
            parse2Event(subPackageString);

        }else if(packageIndex == "01"){
            packageString.clear();
            packageString.append(subPackageString);

        }else if(packageIndex == "02"){
            packageString.append(rs.remainingString());

        }else if(packageIndex == "03"){
            packageString.append(rs.remainingString());
            parse2Event(packageString);
            packageString.clear();
        }
    }
    return true;
}

void RecvPackageParse::disableUpload() {
    enable = false;
}

void RecvPackageParse::parse2Event(string& completePackageString) {
    LOG_INFO << "packageString: " << completePackageString;

    string hciType, subType, packageIndex;
    ReadBinaryString rs(completePackageString);
    rs.read2Byte().readByte(hciType).readByte(subType).readByte(packageIndex);

    if(hciType == "91" && subType == "88"){         //扫描结果上报
        ScanResult(rs.remainingString()).postEvent();

    }else if(hciType == "91" && subType == "9A"){   //网关地址分配成功
        GateWayIndexAck().postEvent();

    }else if(hciType == "91" && subType == "B2"){   //节点配置完成
        NodeAddressAssignAck(rs.remainingString()).postEvent();

    }else if(hciType == "91" && subType == "82"){   //绑定成功
        BindResult(rs.remainingString()).postEvent();

    }else if(hciType == "91" && subType == "81"){   //开关命令上报，灯亮度上报
        string opcode;
        rs.read2Byte().read2Byte().read2Byte(opcode);
        if(opcode == "8204"){           //开关状态
            rs.rollBack(6);
            LightOnOffStatus(rs.remainingString()).postEvent();

        }else if(opcode == "824E"){    //亮度状态
            rs.rollBack(6);
            LightBrightStatus(rs.remainingString()).postEvent();

        }else if(opcode == "8260"){     //亮度、色温状态上报
            rs.rollBack(6);
            LightBrightColorTemperature(rs.remainingString()).postEvent();

        }else if(opcode == "804A"){     //解绑消息
            rs.rollBack(6);
            UnBindResult(rs.remainingString()).postEvent();
        }
    }
}


