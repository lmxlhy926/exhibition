//
// Created by 78472 on 2022/6/15.
//

#include "downBinaryCmd.h"
#include "log/Logging.h"
#include "../parameter.h"

bool DownBinaryCmd::transAndSendCmd(QData &controlData){
    unsigned char buf[100]{};
    size_t size = getBinary(controlData, buf, sizeof buf);
    return DownBinaryUtil::serialSend(buf, static_cast<int>(size));
}

size_t DownBinaryCmd::getBinary(QData &controlData, unsigned char *buf, size_t bufSize) {
    LOG_INFO << "controlData: " << controlData.toJsonString();
    string command  = controlData.getString("command");

    if(command == SCAN){    //扫描命令
        return LightScan().getBinary(buf, bufSize);

    }else if(command == SCANEND){   //扫描结束命令
        return LightScanEnd().getBinary(buf, bufSize);

    }else if(command == CONNECT){   //连接命令
        string deviceSn = controlData.getString("deviceSn");
        return LightConnect(deviceSn).getBinary(buf, bufSize);

    }else if(command == ASSIGN_GATEWAY_ADDRESS){    //分配网关地址
        return LightGatewayAddressAssign().getBinary(buf, bufSize);

    }else if(command == ASSIGN_NODE_ADDRESS){   //分配节点地址
        string nodeAddress = controlData.getString("nodeAddress");
        return LightNodeAddressAssign(nodeAddress).getBinary(buf, bufSize);

    }else if(command == BIND){      //绑定
        return LightBind().getBinary(buf, bufSize);

    }else if(command == UNBIND){    //解绑
        string deviceSn = controlData.getString("deviceSn");
        return LightUnBind(deviceSn).getBinary(buf, bufSize);

    }else if(command == POWER){     //开关
        return LightOnOff(controlData).getBinary(buf, bufSize);

    }else if(command == LUMINANCE){     //亮度
        return LightLuminance(controlData).getBinary(buf, bufSize);

    }else if(command == COLORTEMPERATURE){  //色温
        return LightColorTem(controlData).getBinary(buf, bufSize);
    }

    return 0;
}


