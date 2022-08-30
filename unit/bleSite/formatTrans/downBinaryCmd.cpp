//
// Created by 78472 on 2022/6/15.
//

#include "downBinaryCmd.h"
#include "log/Logging.h"
#include "../parameter.h"
#include "downBinaryFlowControl.h"

bool DownBinaryCmd::transAndSendCmd(QData &controlData){
    string binaryString = getBinaryString(controlData);
    downBinaryFlowControl::getInstance()->push(binaryString);
    return true;
}

string DownBinaryCmd::getBinaryString(QData &controlData) {
    LOG_INFO << "controlData: " << controlData.toJsonString();
    string command  = controlData.getString("command");

    if(command == SCAN){    //扫描命令
        return LightScan().getBinaryString();

    }else if(command == SCANEND){   //扫描结束命令
        return LightScanEnd().getBinaryString();

    }else if(command == CONNECT){   //连接命令
        string deviceSn = controlData.getString("deviceSn");
        return LightConnect(deviceSn).getBinaryString();

    }else if(command == ASSIGN_GATEWAY_ADDRESS){    //分配网关地址
        return LightGatewayAddressAssign().getBinaryString();

    }else if(command == ASSIGN_NODE_ADDRESS){   //分配节点地址
        string nodeAddress = controlData.getString("nodeAddress");
        return LightNodeAddressAssign(nodeAddress).getBinaryString();

    }else if(command == BIND){      //绑定
        return LightBind().getBinaryString();

    }else if(command == UNBIND){    //解绑
        string deviceSn = controlData.getString("deviceSn");
        return LightUnBind(deviceSn).getBinaryString();

    }else if(command == GROUP){     //分组
        string deviceSn = controlData.getString("deviceSn");
        string groupName = controlData.getString("groupName");
        return LightGroup(deviceSn, groupName).getBinaryString();

    }else if(command == POWER){     //开关
        return LightOnOff(controlData).getBinaryString();

    }else if(command == LUMINANCE){     //亮度
        return LightLuminance(controlData).getBinaryString();

    }else if(command == COLORTEMPERATURE){  //色温
        return LightColorTem(controlData).getBinaryString();
    }

    return string();
}


