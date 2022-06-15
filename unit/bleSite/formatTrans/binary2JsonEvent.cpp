//
// Created by 78472 on 2022/6/13.
//

#include "binary2JsonEvent.h"
#include <iostream>

#define HIGH4BITS(uc) (((uc) >> 4) & 0x0f)
#define LOW4BITS(uc)  (((uc) >> 0) & 0x0f)

string CharArray2BinaryString::getBinaryString(const unsigned char *binaryStream, size_t size) {
    const char digitsHex[] = "0123456789ABCDEF";
    string data;
    for(size_t i = 0; i < size; i++){
        unsigned char chr = binaryStream[i];
        data.append(1, digitsHex[HIGH4BITS(chr)]);
        data.append(1, digitsHex[LOW4BITS(chr)]);
    }
    return data;
}

ReadBinaryString &ReadBinaryString::readByte(string &dest) {
    if(binaryString_.size() - readIndex >= 2){
        dest = binaryString_.substr(readIndex, 2);
        readIndex += 2;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::readByte() {
    if(binaryString_.size() - readIndex >= 2){
        readIndex += 2;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::read2Byte(string &dest) {
    if(binaryString_.size() - readIndex >= 4){
        dest = binaryString_.substr(readIndex, 4);
        readIndex += 4;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::read2Byte() {
    if(binaryString_.size() - readIndex >= 4){
        readIndex += 4;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::readBytes(string &dest, int readBytesNum) {
    if(binaryString_.size() - readIndex >= readBytesNum * 2){
        dest = binaryString_.substr(readIndex, readBytesNum * 2);
        readIndex += readBytesNum * 2;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::readBytes(int readBytesNum) {
    if(binaryString_.size() - readIndex >= readBytesNum * 2){
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

string BinaryString2JsonEvent::getJsonStringEvent() {
    string hciType, subType, packageIndex;
    ReadBinaryString rs(binaryString_);
    rs.read2Byte().readByte(hciType).readByte(subType).readByte(packageIndex);

    if(hciType == "91" && subType == "81"){
        string opcode;
        rs.read2Byte().read2Byte().read2Byte(opcode);
        if(opcode == "8204"){
            rs.rollBack(6);
            return LightOnOffStatus(rs.remainingString()).construct();

        }else if(opcode == "824E"){
            rs.rollBack(6);
            return LightBrightStatus(rs.remainingString()).construct();
        }
    }
    return string();
}
