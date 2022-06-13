//
// Created by 78472 on 2022/6/13.
//

#ifndef EXHIBITION_UPTRANSFORM_H
#define EXHIBITION_UPTRANSFORM_H


//第一步，二进制流转换为字符流

//第二步，按照格式提取字符

//第三步， 根据关键字提取信息，组装信息

#include <string>
using namespace std;

class ReadString{
public:
    static void readByte(string& data, string& dest){
        if(!data.empty()){
            dest = data.substr(0, 1);
            data = data.substr(1);
        }
    }

    static void read2Byte(string& data, string& dest){
        if(data.size() >= 2){
            dest = data.substr(0, 2);
            data = data.substr(2);
        }
    }

    static void readBytes(string& data, string& dest, int readBytesNum){
        if(data.size() >= readBytesNum){
            dest = data.substr(0, readBytesNum);
            data = data.substr(readBytesNum);
        }
    }
};



class LightOnOffStatus{
private:
    string sourceData;
    string unicast_address;
    string group_address;
    string opcode;
    string present_onOff;
    string target_onOff;
    string remaining_time;
public:
    explicit LightOnOffStatus(string data) : sourceData(std::move(data)){
        init();
    }

    string construct(){
        return string();
    }

private:
    void init(){
        ReadString::read2Byte(sourceData, unicast_address);
        ReadString::read2Byte(sourceData, group_address);
        ReadString::readByte(sourceData, opcode);
        ReadString::readByte(sourceData, present_onOff);
        ReadString::readByte(sourceData, target_onOff);
        ReadString::read2Byte(sourceData, remaining_time);
    }
};


class LightBrightStatus{
private:
    string sourceData;
    string unicast_address;
    string group_address;
    string opcode;
    string present_lightness;
    string target_lightness;
    string remaining_time;
public:
    explicit LightBrightStatus(string data) : sourceData(std::move(data)){
        init();
    }

    string construct(){
        return string();
    }

private:
    void init(){
        ReadString::read2Byte(sourceData, unicast_address);
        ReadString::read2Byte(sourceData, group_address);
        ReadString::readByte(sourceData, opcode);
        ReadString::read2Byte(sourceData, present_lightness);
        ReadString::read2Byte(sourceData, target_lightness);
        ReadString::readByte(sourceData, remaining_time);
    }
};







#endif //EXHIBITION_UPTRANSFORM_H
