//
// Created by 78472 on 2022/6/15.
//

#include "downBinaryCmd.h"
#include "../parameter.h"
#include "log/Logging.h"

size_t DownBinaryCmd::getBinary(QData &data, unsigned char *buf, size_t bufSize) {
    LOG_INFO << data.toJsonString();
    string command  = data.getString("command");

    if(command == SCAN){
        return LightScan().getBinary(buf, bufSize);

    }else if(command == SCANEND){
        return LightScanEnd().getBinary(buf, bufSize);

    }else if(command == CONNECT){
        string deviceSn = data.getString("deviceSn");
        return LightConnect(deviceSn).getBinary(buf, bufSize);

    }else if(command == ASSIGN_GATEWAY_ADDRESS){
        return LightGatewayAddressAssign().getBinary(buf, bufSize);

    }else if(command == ASSIGN_NODE_ADDRESS){
        string nodeAddress = data.getString("nodeAddress");
        return LightNodeAddressAssign(nodeAddress).getBinary(buf, bufSize);

    }else if(command == BIND){
        return LightBind().getBinary(buf, bufSize);

    }else if(command == UNBIND){
        string deviceSn = data.getString("deviceSn");
        return LightUnBind(deviceSn).getBinary(buf, bufSize);

    }else if(command == POWER){
        return LightOnOff(data).getBinary(buf, bufSize);

    }else if(command == LUMINANCE){
        return LightLuminance(data).getBinary(buf, bufSize);

    }else if(command == COLORTEMPERATURE){
        return LightColorTem(data).getBinary(buf, bufSize);
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
