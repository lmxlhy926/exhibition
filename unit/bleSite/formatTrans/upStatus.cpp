//
// Created by 78472 on 2022/7/4.
//

#include "upStatus.h"

ReadBinaryString &ReadBinaryString::readByte(string &dest) {
    if(avail() >= 2){
        dest = binaryString_.substr(readIndex, 2);
        readIndex += 2;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::readByte() {
    if(avail() >= 2){
        readIndex += 2;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::read2Byte(string &dest) {
    if(avail() >= 4){
        dest = binaryString_.substr(readIndex, 4);
        readIndex += 4;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::read2Byte() {
    if(avail() >= 4){
        readIndex += 4;
    }
    return *this;
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