//
// Created by 78472 on 2022/6/13.
//

#ifndef EXHIBITION_UPBINAYCMD_H
#define EXHIBITION_UPBINAYCMD_H

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>

using namespace std;

class UpBinaryCmd{
    static mutex mutex_;
    static string packageString;
public:
    //接收串口返回，解析返回数据，产生相应的事件
    static bool bleReceiveFunc(unsigned char* binaryStream, int size);
};


#endif //EXHIBITION_UPBINAYCMD_H
