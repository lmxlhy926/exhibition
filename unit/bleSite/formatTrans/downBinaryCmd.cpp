//
// Created by 78472 on 2022/6/15.
//

#include "downBinaryCmd.h"
#include "../parameter.h"

size_t DownBinaryCmd::getBinary(QData &data, unsigned char *buf, size_t bufSize) {
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

    }else if(pseudoCommand == UNBIND){
        string deviceSn = data.getString("deviceSn");
        return LightUnBind(deviceSn).getBinary(buf, bufSize);
    }

    return 0;
}

bool DownBinaryCmd::serialSend(unsigned char *buf, int size){
    return DownBinaryUtil::serialSend(buf, static_cast<int>(size));
}

bool DownBinaryCmd::transAndSendCmd(QData &controlData){
    unsigned char buf[100]{};
    size_t size = getBinary(controlData, buf, 100);
    return serialSend(buf, static_cast<int>(size));
}
