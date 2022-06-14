//
// Created by 78472 on 2022/6/13.
//

#include "binaryStatus2Json.h"

string CharArray2String::getBinaryString(const unsigned char *binaryStream, size_t size) {
    const char digitsHex[] = "0123456789ABCDEF";
    string data;
    for(size_t i = 0; i < size; i++){
        unsigned char chr = binaryStream[i];
        data.append(1, digitsHex[chr & 0x0f]);
        data.append(1, digitsHex[chr & 0xf0]);
    }
    return data;
}

ReadString &ReadString::readByte(string &dest) {
    if(binaryString_.size() - readIndex >= 1){
        dest = binaryString_.substr(readIndex, 1);
        readIndex++;
    }
    return *this;
}

ReadString &ReadString::readByte() {
    if(binaryString_.size() - readIndex >= 1){
        readIndex++;
    }
    return *this;
}

ReadString &ReadString::read2Byte(string &dest) {
    if(binaryString_.size() - readIndex >= 2){
        dest = binaryString_.substr(readIndex, 2);
        readIndex += 2;
    }
    return *this;
}

ReadString &ReadString::read2Byte() {
    if(binaryString_.size() - readIndex >= 2){
        readIndex += 2;
    }
    return *this;
}

ReadString &ReadString::readBytes(string &dest, int readBytesNum) {
    if(binaryString_.size() - readIndex >= readBytesNum){
        dest = binaryString_.substr(readIndex, readBytesNum);
        readIndex += readBytesNum;
    }
    return *this;
}

ReadString &ReadString::readBytes(int readBytesNum) {
    if(binaryString_.size() - readIndex >= readBytesNum){
        readIndex += readBytesNum;
    }
    return *this;
}

string LightOnOffStatus::construct() {
    return std::string();
}

void LightOnOffStatus::init() {
    ReadString rs(sourceData);
    rs.read2Byte(unicast_address);
    rs.read2Byte(group_address);
    rs.readByte(opcode);
    rs.readByte(present_onOff);
    rs.readByte(target_onOff);
    rs.read2Byte(remaining_time);
}

string LightBrightStatus::construct() {
    return std::string();
}

void LightBrightStatus::init() {
    ReadString rs(sourceData);
    rs.read2Byte(unicast_address);
    rs.read2Byte(group_address);
    rs.readByte(opcode);
    rs.read2Byte(present_lightness);
    rs.read2Byte(target_lightness);
    rs.readByte(remaining_time);
}

string BinaryStringCmdUp::getResStatusString() {
    string hciType, subType, packageIndex;
    ReadString rs(binaryString_);
    rs.read2Byte().readByte(hciType).readByte(subType).readByte(packageIndex);

    if(hciType == "91" && subType == "81"){
        string opcode;
        rs.read2Byte().read2Byte().readByte(opcode);
        if(opcode == "8204"){
            rs.rollBack(5);
            return LightOnOffStatus(rs.remainingString()).construct();
        }else if(opcode == "824E"){
            rs.rollBack(5);
            return LightBrightStatus(rs.remainingString()).construct();
        }
    }
}
