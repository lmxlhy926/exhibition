//
// Created by 78472 on 2022/6/15.
//

#include "controlCmd.h"
#include "bleConfigParam.h"

void LightScanAddDel::init(QData &data) {
    pseudoCommand  = data.getData("request").getString("command");
    device_id = data.getData("request").getString("device_id");
}

size_t LightScanAddDel::getBinary(unsigned char *buf, size_t bufSize) {
    qlibc::QData thisBleConfigData = bleConfigParam::getInstance()->getBleParamData();
    string binaryString;

    if(pseudoCommand == "scan"){
        binaryString = "E9FF00";

    }else if(pseudoCommand == "addDevice"){
        binaryString = "E9FF08";
        binaryString += device_id;

    }else if(pseudoCommand == "deleteDevice"){

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


