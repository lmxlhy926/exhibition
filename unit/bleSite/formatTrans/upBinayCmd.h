//
// Created by 78472 on 2022/6/13.
//

#ifndef EXHIBITION_UPBINAYCMD_H
#define EXHIBITION_UPBINAYCMD_H

#include <string>
#include <iostream>
#include <sstream>

using namespace std;

class UpBinaryCmd{
public:
    //接收串口返回，产生相应的事件
    static bool parseAndGenerateEvent(unsigned char* binaryStream, int size);
};


#endif //EXHIBITION_UPBINAYCMD_H
