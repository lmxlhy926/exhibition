//
// Created by 78472 on 2022/6/13.
//

#ifndef EXHIBITION_BINARY2JSONEVENT_H
#define EXHIBITION_BINARY2JSONEVENT_H

#include <string>
#include <iostream>
#include <sstream>
using namespace std;

class Binary2JsonEvent{
public:
    explicit Binary2JsonEvent() = default;

    //接收串口返回，产生相应的事件
    static bool binary2JsonEvent(unsigned char* binaryStream, int size);

    //打印收到的响应
    static void printBinaryString(string& str);

    static void postEvent(string& statusString);
};


#endif //EXHIBITION_BINARY2JSONEVENT_H
