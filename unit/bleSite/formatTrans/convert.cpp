//
// Created by 78472 on 2022/6/14.
//

#include "convert.h"
#include "jsonCmd2Binary.h"
#include "binaryStatus2Json.h"

size_t getBleControlBinary(qlibc::QData& data, unsigned char* buf, size_t bufSize){
    string pseudoCommand  = data.getData("request").getString("command");

    string binaryControlString;
    if(pseudoCommand == "scan"){
        binaryControlString = "E9FF00";

    }else if(pseudoCommand == "addDevice"){
        binaryControlString = "E9FF08";
        binaryControlString += data.getData("request").getString("device_id");

    }else if(pseudoCommand == "deleteDevice"){

    }
    std::cout << "==>binaryControlString: " << binaryControlString << std::endl;

    size_t size = JsonCmd2Binary::binaryString2binary(binaryControlString, buf, bufSize);

    for(int i = 0; i < bufSize; i++){
        printf("==>%2x\n", buf[i]);
    }
    return size;
}


size_t getBleCommandBinaray(qlibc::QData& data, unsigned char* buf, size_t bufSize){
    std::cout << "===>getBleCommandBinaray: " << data.toJsonString() << std::endl;
    JsonCmd2Binary cmd(data);
    return cmd.getBinary(buf, bufSize);
}


string binaryCommand2JsonString(unsigned char* buf, size_t bufSize){
    string binaryString = CharArray2String::getBinaryString(buf, bufSize);
    BinaryStringCmdUp bscp(binaryString);
    return bscp.getResStatusString();
}