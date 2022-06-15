//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_JSONCMD2BINARY_H
#define EXHIBITION_JSONCMD2BINARY_H


#include <iostream>
#include "qlibc/QData.h"
#include "bleConfigParam.h"

class JsonCmd2Binary{
private:
    string pseudoCommand;
    string address;
    string device_id;
public:
    explicit JsonCmd2Binary(qlibc::QData& request){
       init(request);
    }

    //获取二进制格式蓝牙命令
    size_t getBinary(unsigned char* buf, size_t bufSize);

    //获取构造的字符串形式的二进制数据
    static string getBinaryString(qlibc::QData& bleConfigData);

    //将字符串转换为二进制格式
    static size_t binaryString2binary(string& binaryString, unsigned char* buf, size_t size);
private:
    class BinaryBuf{
    private:
        unsigned char* binaryBuf_;
        size_t size_ = 0;
        int count_ = 0;
    public:
        explicit BinaryBuf(unsigned char* buffer, size_t size)
                : binaryBuf_(buffer), size_(size){}

        void append(string& charString){
            int charInt;
            try{
                charInt =  std::stoi(charString, nullptr, 16);
            }catch(std::exception& e){
                charInt = 0;
            }
            if(count_ < size_){
                binaryBuf_[count_] = static_cast<unsigned char>(charInt);
                count_++;
            }
        }

        size_t size() const{ return count_; }
    };

    void init(qlibc::QData& request);

    void set_light_turnOnOff(qlibc::QData& lightData);
};


#endif //EXHIBITION_JSONCMD2BINARY_H
