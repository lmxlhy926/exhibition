//
// Created by 78472 on 2022/6/15.
//

#include "controlCmd.h"
#include "bleConfigParam.h"
#include "../paramConfig.h"

void LightScanAddDel::init(QData &data) {
    pseudoCommand  = data.getData("request").getString("command");
    device_id = data.getData("request").getString("device_id");
}

size_t LightScanAddDel::getBinary(unsigned char *buf, size_t bufSize) {
    qlibc::QData thisBleConfigData = bleConfigParam::getInstance()->getBleParamData();
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
    qlibc::QData thisBleConfigData = bleConfigParam::getInstance()->getBleParamData();
    thisBleConfigData.asValue()["commonBase"]["param"]["ADDRESS_DEST"] = address;
    thisBleConfigData.asValue()["commonBase"]["param"]["OPERATION"] = pseudoCommand;

    string binaryString = getBinaryString(thisBleConfigData);
    return binaryString2binary(binaryString, buf, bufSize);
}


