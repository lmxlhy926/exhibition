//
// Created by 78472 on 2022/6/15.
//

#include "downCmd.h"
#include "../parameter.h"


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


void LightScanAddDel::init(QData &data) {
    pseudoCommand  = data.getData("request").getString("command");
    device_id = data.getData("request").getString("device_id");
}

size_t LightScanAddDel::getBinary(unsigned char *buf, size_t bufSize) {
    qlibc::QData thisBleConfigData = bleConfig::getInstance()->getBleParamData();
    string binaryString;

    if(pseudoCommand == SCAN){
        binaryString = "E9FF00";

    }else if(pseudoCommand == SCANEND){
        binaryString = "E9FF01";

    }else if(pseudoCommand == CONNECT){
        binaryString = "E9FF08" + device_id;

    }else if(pseudoCommand == ASSIGN_GATEWAY_ADDRESS){
        binaryString = "E9FF091112131415161718191A1B1C1D1E1F20000000112233440100";

    }else if(pseudoCommand == ASSIGN_NODE_ADDRESS){
        binaryString = "E9FF0A1112131415161718191A1B1C1D1E1F20000000112233440200";

    }else if(pseudoCommand == BIND){
        binaryString = "E9FF0B00000060964771734FBD76E3B40519D1D94A48";
    }

    return binaryString2binary(binaryString, buf, bufSize);
}

void LightOnOff::init(qlibc::QData &data) {
    pseudoCommand  = data.getData("request").getString("command");
    address = data.getData("request").getString("device_id");
}

size_t LightOnOff::getBinary(unsigned char *buf, size_t bufSize) {
    qlibc::QData thisBleConfigData = bleConfig::getInstance()->getBleParamData();
    thisBleConfigData.asValue()["commonBase"]["param"]["ADDRESS_DEST"] = address;
    thisBleConfigData.asValue()["commonBase"]["param"]["OPERATION"] = pseudoCommand;

    string binaryString = getBinaryString(thisBleConfigData);
    return binaryString2binary(binaryString, buf, bufSize);
}


