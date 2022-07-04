//
// Created by 78472 on 2022/7/4.
//

#ifndef EXHIBITION_UPSTATUS_H
#define EXHIBITION_UPSTATUS_H

#include <string>
using namespace std;

class ReadBinaryString{
private:
    string binaryString_;
    size_t readIndex = 0;
public:
    explicit ReadBinaryString(string binaryString)
            : binaryString_(std::move(binaryString)){}

    ReadBinaryString& readByte(string& dest);
    ReadBinaryString& readByte();
    ReadBinaryString& read2Byte(string& dest);
    ReadBinaryString& read2Byte();
    ReadBinaryString& readBytes(string& dest, int readBytesNum);
    ReadBinaryString& readBytes(int readBytesNum);

    int avail(){
        return static_cast<int>(binaryString_.size() - readIndex);
    }

    void reset() {readIndex = 0; }

    void rollBack(size_t nBytes) {
        if(readIndex - nBytes * 2 >= 0)
            readIndex -= nBytes * 2;
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


#endif //EXHIBITION_UPSTATUS_H
