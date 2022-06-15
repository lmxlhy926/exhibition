//
// Created by 78472 on 2022/6/14.
//

#include "convert.h"
#include "jsonCmd2Binary.h"
#include "binary2JsonEvent.h"


size_t bleJsonCmd2Binaray(qlibc::QData& data, unsigned char* buf, size_t bufSize){
    std::cout << "===>getBleCommandBinaray: " << data.toJsonString() << std::endl;
    JsonCmd2Binary cmd(data);
    return cmd.getBinary(buf, bufSize);
}

string binaryCommand2JsonStringEvent(unsigned char* buf, size_t bufSize){
    string binaryString = CharArray2BinaryString::getBinaryString(buf, bufSize);
    BinaryString2JsonEvent bs2je(binaryString);
    return bs2je.getJsonStringEvent();
}