//
// Created by 78472 on 2022/6/14.
//

#include "convert.h"
#include "jsonCmd2Binary.h"
#include "binary2JsonEvent.h"
#include "controlCmd.h"
#include "../paramConfig.h"


size_t bleJsonCmd2Binaray(qlibc::QData& data, unsigned char* buf, size_t bufSize){
    std::cout << "===>getBleCommandBinaray: " << data.toJsonString() << std::endl;
    string pseudoCommand  = data.getData("request").getString("command");

    if(pseudoCommand == "turnOn" || pseudoCommand == "turnOff"){
        return LightOnOff(data).getBinary(buf, bufSize);

    }else if(pseudoCommand == SCAN || pseudoCommand == SCANEND ||
             pseudoCommand == CONNECT || pseudoCommand == ASSIGN_GATEWAY_ADDRESS ||
             pseudoCommand == ASSIGN_NODE_ADDRESS || pseudoCommand == BIND){
        return LightScanAddDel(data).getBinary(buf, bufSize);
    }

    return 0;
}

string binaryCommand2JsonStringEvent(unsigned char* buf, size_t bufSize){
    string binaryString = CharArray2BinaryString::getBinaryString(buf, bufSize);
    BinaryString2JsonEvent bs2je(binaryString);
    return bs2je.getJsonStringEvent();
}