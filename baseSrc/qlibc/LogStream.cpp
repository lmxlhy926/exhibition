//
// Created by 78472 on 2022/5/30.
//

#include "LogStream.h"
#include <algorithm>

namespace qlibc{
    const char digits[] = "9876543210123456789";
    const char* zero = digits + 9;
    const char digitsHex[] = "0123456789ABCDEF";

    //Efficient Integer to String Conversions;
    template<typename T>
    size_t convert(char buf[], T value){
        T i = value;    //待转换整数值
        char* p = buf;  //指向buf空间首地址

        //逆序转换
        do{
            int lsd = i % 10;
            *p++ = zero[lsd];
            i /= 10;
        }while(i != 0);

        //增加正负号以及字符串结尾'\0'
        if(value < 0)
            *p++ = '-';
        *p = '\0';

        //转换为正序，返回所占用的空间大小
        std::reverse(buf, p);
        return p - buf;
    }

    //将16进制数转换为字符串
    size_t convertHex(char buf[], uintptr_t value){
        uintptr_t i = value;
        char* p = buf;

        do{
            int lsd = static_cast<int>(i % 16);
            *p++ = digitsHex[lsd];
            i /= 16;
        }while(i != 0);

        *p = '\0';
        std::reverse(buf, p);
        return p -buf;
    }

    //将整数转换为字符串，并写入到buffer_中
    template<typename T>
    void LogStream::formatInteger(T v) {
        if(buffer_.avail() > KMaxNumericSize){
            size_t len = convert(buffer_.current(), v);
            buffer_.add(len);
        }
    }

    LogStream& LogStream::operator<<(bool v) {
        buffer_.append(v ? "1" : "0", 1);
        return *this;
    }

    LogStream& LogStream::operator<<(char v) {
       buffer_.append(&v, 1);
       return *this;
    }

    LogStream& LogStream::operator<<(short v) {
       *this << static_cast<int>(v);
       return *this;
    }

    LogStream& LogStream::operator<<(unsigned short v) {
        *this << static_cast<int>(v);
        return *this;
    }

    LogStream& LogStream::operator<<(int) {

    }

    LogStream& LogStream::operator<<(unsigned int) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(long) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(unsigned long) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(long long int) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(unsigned long long int) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(float) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(double) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(const void *) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(const char *) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(const unsigned char *) {
        return <#initializer#>;
    }

    LogStream& LogStream::operator<<(const string &) {
        return <#initializer#>;
    }

    void LogStream::append(const char *data, size_t len) {

    }

    const LogStream::Buffer &LogStream::buffer() const {
        return <#initializer#>;
    }

    void LogStream::resetBuffer() {

    }


}
