//
// Created by 78472 on 2022/7/4.
//

#include "statusEvent.h"

EventTable* EventTable::eventTable = nullptr;

ReadBinaryString &ReadBinaryString::readByte(string &dest) {
   return readBytes(dest, 1);
}

ReadBinaryString &ReadBinaryString::readByte() {
   return readBytes(1);
}

ReadBinaryString &ReadBinaryString::read2Byte(string &dest) {
   return readBytes(dest, 2);
}

ReadBinaryString &ReadBinaryString::read2Byte() {
   return readBytes(2);
}

ReadBinaryString &ReadBinaryString::readBytes(string &dest, int readBytesNum) {
    if(avail() >= readBytesNum * 2){
        dest = binaryString_.substr(readIndex, readBytesNum * 2);
        readIndex += readBytesNum * 2;
    }
    return *this;
}

ReadBinaryString &ReadBinaryString::readBytes(int readBytesNum) {
    if(avail() >= readBytesNum * 2){
        readIndex += readBytesNum * 2;
    }
    return *this;
}

int ReadBinaryString::avail(){
    return static_cast<int>(binaryString_.size() - readIndex);
}
