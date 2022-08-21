//
// Created by 78472 on 2022/7/4.
//

#include "statusEvent.h"

EventTable* EventTable::eventTable = nullptr;

ReadBinaryString &ReadBinaryString::readByte(string &dest) {
   return readBytes(dest, 1);
}

ReadBinaryString &ReadBinaryString::readByte() {
   return readBytes(1);
}

ReadBinaryString &ReadBinaryString::read2Byte(string &dest) {
   return readBytes(dest, 2);
}

ReadBinaryString &ReadBinaryString::read2Byte() {
   return readBytes(2);
}

ReadBinaryString &ReadBinaryString::readBytes(string &dest, int readBytesNum) {
    if(avail() >= readBytesNum * 2){
        dest = binaryString_.substr(readIndex, readBytesNum * 2);
        readIndex += readBytesNum * 2;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::readBytes(int readBytesNum) {
    if(avail() >= readBytesNum * 2){
        readIndex += readBytesNum * 2;
    }
    return *this;
}

int ReadBinaryString::avail(){
    return static_cast<int>(binaryString_.size() - readIndex);
}

string LightOnOffStatus::construct() {
    return
            unicast_address + '-' +
            group_address   + '-' +
            opcode          + '-' +
            present_onOff   + '-' +
            target_onOff    + '-' +
            remaining_time;
}

void LightOnOffStatus::init() {
    ReadBinaryString rs(sourceData);
    rs.read2Byte(unicast_address);
    rs.read2Byte(group_address);
    rs.read2Byte(opcode);
    rs.readByte(present_onOff);
    rs.readByte(target_onOff);
    rs.readByte(remaining_time);
}

string LightBrightStatus::construct() {
    return
            unicast_address + '-' +
            group_address   + '-' +
            opcode          + '-' +
            present_lightness   + '-' +
            target_lightness    + '-' +
            remaining_time;
}

void LightBrightStatus::init() {
    ReadBinaryString rs(sourceData);
    rs.read2Byte(unicast_address);
    rs.read2Byte(group_address);
    rs.read2Byte(opcode);
    rs.read2Byte(present_lightness);
    rs.read2Byte(target_lightness);
    rs.readByte(remaining_time);
}


void PostStatusEvent::operator()() {
    string hciType, subType, packageIndex;
    ReadBinaryString rs(statusString);
    rs.read2Byte().readByte(hciType).readByte(subType).readByte(packageIndex);

    if(hciType == "91" && subType == "88"){         //扫描结果上报
        ScanResult sr(rs.remainingString());
        sr.postEvent();

    }else if(hciType == "91" && subType == "B2"){   //节点配置完成
        NodeAddressAssignAck nodeAck(rs.remainingString());
        nodeAck.postEvent();

    }else if(hciType == "91" && subType == "82"){   //绑定成功
        BindResult br(rs.remainingString());
        br.postEvent();

    }else if(hciType == "91" && subType == "81"){   //开关命令上报，灯亮度上报
        string opcode;
        rs.read2Byte().read2Byte().read2Byte(opcode);
        if(opcode == "8204"){           //开关状态
            rs.rollBack(6);
            LOG_GREEN << "<<===: LightOnOffStatus: " << LightOnOffStatus(rs.remainingString()).construct();

        }else if(opcode == "824E"){    //亮度状态
            rs.rollBack(6);
            LOG_GREEN << "<<===: LightBrightStatus: " << LightBrightStatus(rs.remainingString()).construct();

        }else if(opcode == "804A"){     //解绑消息
            rs.rollBack(6);
            UnBindResult unBindResult(rs.remainingString());
            unBindResult.postEvent();
        }
    }
}
