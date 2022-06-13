//
// Created by 78472 on 2022/6/13.
//

#include "bleStatus.h"



string charArray2String(const char* binaryStream, int size){
    const char digitsHex[] = "0123456789ABCDEF";
    string data;
    for(int i = 0; i < size; i++){
        char chr = binaryStream[i];
        data.append(1, digitsHex[chr & 0x0f]);
        data.append(1, digitsHex[chr & 0xf0]);
    }
    return data;
}