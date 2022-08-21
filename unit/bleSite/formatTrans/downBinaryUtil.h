//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_DOWNBINARYUTIL_H
#define EXHIBITION_DOWNBINARYUTIL_H


#include <iostream>
#include "qlibc/QData.h"
#include "bleConfig.h"

class BinaryBuf{
private:
    unsigned char* binaryBuf_;
    size_t size_ = 0;
    int count_ = 0;
public:
    explicit BinaryBuf(unsigned char* buffer, size_t size)
            : binaryBuf_(buffer), size_(size){}

    void append(const string& charString){
        int charInt;
        try{
            charInt =  std::stoi(charString, nullptr, 16);
        }catch(std::exception& e){
            charInt = 0;
        }
        if(count_ < size_){
            binaryBuf_[count_++] = static_cast<unsigned char>(charInt);
        }
    }

    void append(char c){
        if(count_ < size_){
            binaryBuf_[count_++] = c;
        }
    }

    size_t size() const{ return count_; }
};


/*
 * 1. 将命令的字符串表示，转换为二进制表示
 * 2. 使用串口发送数据
 * 3. 打印串口发送的数据
 */
class DownBinaryUtil{
    static std::mutex sendMutex;
public:
    /**
     * 将二进制命令的字符串表示转换为二进制表示
     * @param binaryString      二进制命令的字符串表示
     * @param buf               存放二进制命令的数组
     * @param size              提供的数组容量
     * @return                  二进制命令实际占用的字节数
     */
    static size_t binaryString2binary(string& binaryString, unsigned char* buf, size_t size);

    //串口发送数据
    static bool serialSend(unsigned char *buf, int size);

private:
    //打印发送的二进制命令
    static void printSendBinary(unsigned char *buf, int size);
};


#endif //EXHIBITION_DOWNBINARYUTIL_H
