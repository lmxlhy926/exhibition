//
// Created by 78472 on 2022/6/15.
//

#include "lightControlCmd.h"
#include "../parameter.h"


size_t bleJsonCmd2Binaray(qlibc::QData& data, unsigned char* buf, size_t bufSize){
    std::cout << "===>getBleCommandBinaray: " << data.toJsonString() << std::endl;
    string pseudoCommand  = data.getString("command");

    if(pseudoCommand == "turnOn" || pseudoCommand == "turnOff"){
        return LightOnOff(data).getBinary(buf, bufSize);

    }else if(pseudoCommand == SCAN){
        return LightScan().getBinary(buf, bufSize);

    }else if(pseudoCommand == SCANEND){
        return LightScanEnd().getBinary(buf, bufSize);

    }else if(pseudoCommand == CONNECT){
        string deviceSn = data.getString("deviceSn");
        return LightConnect(deviceSn).getBinary(buf, bufSize);

    }else if(pseudoCommand == ASSIGN_GATEWAY_ADDRESS){
        return LightGatewayAddressAssign().getBinary(buf, bufSize);

    }else if(pseudoCommand == ASSIGN_NODE_ADDRESS){
        string nodeAddress = data.getString("nodeAddress");
        return LightNodeAddressAssign(nodeAddress).getBinary(buf, bufSize);

    }else if(pseudoCommand == BIND){
        return LightBind().getBinary(buf, bufSize);
    }

    return 0;
}



