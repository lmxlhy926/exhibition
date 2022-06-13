//
// Created by 78472 on 2022/5/30.
//

#ifndef EXHIBITION_LOGSTREAM_H
#define EXHIBITION_LOGSTREAM_H

#include <cstdarg>
#include <cstring>
#include <string>

using namespace std;
namespace qlibc{
    const int SmallBufferSize = 1024 * 4;

    template<int SIZE>
    class FixedBuffer{
    private:
        char data_[SIZE]{};         //存储数据的buffer
        char* cur_;                 //记录当前位置
        const char* end() const{    //返回data_末尾位置，[)
            return data_ + sizeof data_;
        }
    public:
        FixedBuffer(): cur_(data_){}    //将当前位置定位到data_的起始位置

        ~FixedBuffer()= default;

        //如果空间足够，写入数据，移动当前位置
        void append(const char* buf, size_t len){
            if(avail() > len){
                memcpy(cur_, buf, len);
                cur_ += len;
            }
        }

        //返回buffer首地址
        const char* data() const{
            return data_;
        }

        //返回buffer已存储的数据长度
        int length() const{
            return cur_ - data_;
        }

        //返回存储数据的当前位置
        char* current(){
            return cur_;
        }

        int avail() const{
            return end() - cur_;
        }

        //移动当前位置
        void add(size_t len){
            cur_ += len;
        }

        //复位当前位置到buffer首地址
        void reset(){
            cur_ = data_;
        }

        //以字符串格式返回data_内容
        string toString() const{
            return string(data_, length());
        }
    };


    //将所有的数据类型全部以字符串的形式写入到Buffer中
    class LogStream {
    public:
        typedef LogStream self;
        typedef FixedBuffer<SmallBufferSize> Buffer;
    private:
        Buffer buffer_;
        static const int KMaxNumericSize = 32;  //数字转换后最多占用的字符数
    private:
        template<typename T>
        void formatInteger(T);

    public:
        self& operator<< (bool v);
        self& operator<<(char);
        self& operator<<(short);
        self& operator<<(unsigned short);
        self& operator<<(int);
        self& operator<<(unsigned int);
        self& operator<<(long);
        self& operator<<(unsigned long);
        self& operator<<(long long);
        self& operator<<(unsigned long long);

        self& operator<<(float);
        self& operator<<(double);

        self& operator<<(const void*);
        self& operator<<(const char*);
        self& operator<<(const unsigned char*);
        self& operator<<(const string&);

        void append(const char* data, size_t len);
        const Buffer& buffer() const;
        void resetBuffer();
    };

}




#endif //EXHIBITION_LOGSTREAM_H
