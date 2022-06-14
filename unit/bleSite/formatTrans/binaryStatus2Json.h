//
// Created by 78472 on 2022/6/13.
//

#ifndef EXHIBITION_BINARYSTATUS2JSON_H
#define EXHIBITION_BINARYSTATUS2JSON_H

#include <string>
using namespace std;

class CharArray2String{
public:
    static string getBinaryString(const unsigned char* binaryStream, size_t size);
};

class ReadString{
private:
    string binaryString_;
    size_t readIndex = 0;
public:
    explicit ReadString(string binaryString)
        : binaryString_(std::move(binaryString)){}

    ReadString& readByte(string& dest);
    ReadString& readByte();
    ReadString& read2Byte(string& dest);
    ReadString& read2Byte();
    ReadString& readBytes(string& dest, int readBytesNum);
    ReadString& readBytes(int readBytesNum);

    void reset() {readIndex = 0; }
    void rollBack(size_t n) {
        if(readIndex - n >= 0)
            readIndex -= n;
    }
    string remainingString(){ return binaryString_.substr(readIndex); }
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
    string construct();
private:
    void init();
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
    string construct();
private:
    void init();
};


class BinaryStringCmdUp{
private:
    string binaryString_;
public:
    explicit BinaryStringCmdUp(string binaryString) : binaryString_(std::move(binaryString)){}

    string getResStatusString();
};


#endif //EXHIBITION_BINARYSTATUS2JSON_H
