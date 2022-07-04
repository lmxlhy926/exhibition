//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_JSONCMD2BINARY_H
#define EXHIBITION_JSONCMD2BINARY_H


#include <iostream>
#include "qlibc/QData.h"
#include "bleConfig.h"

class JsonCmd2Binary{
public:
    /**
     * 获取二进制控制命令，各个具体派生类实现
     * @param buf       存放二进制命令的数组
     * @param bufSize   数组的容量
     * @return
     */
    virtual size_t getBinary(unsigned char* buf, size_t bufSize) = 0;

    /**
     * 获取二进制命令的字符串表示
     * @param bleConfigData 填入参数后的蓝牙格式数据
     * @return  二进制命令的字符串表示
     */
    static string getBinaryString(qlibc::QData& bleConfigData);

    /**
     * 将二进制命令的字符串表示转换为二进制表示
     * @param binaryString  二进制命令的字符串表示
     * @param buf   存放二进制命令的数组
     * @param size  数据大小
     * @return  二进制命令实际占用的字节数
     */
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
};


#endif //EXHIBITION_JSONCMD2BINARY_H
