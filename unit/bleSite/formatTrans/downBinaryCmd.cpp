//
// Created by 78472 on 2022/6/15.
//

#include "downBinaryCmd.h"
#include "log/Logging.h"
#include "../parameter.h"
#include "downBinaryFlowControl.h"

string JsonCmd2Binary::commandPrefix = "E8 FF 00 00 00 00 02 03";

bool DownBinaryCmd::transAndSendCmd(QData &cmdData){
    string binaryString = getBinaryString(cmdData);
    downBinaryFlowControl::getInstance()->push(binaryString);
    return true;
}

string DownBinaryCmd::getBinaryString(QData &cmdData) {
    LOG_INFO << "cmdData: " << cmdData.toJsonString();
    string command  = cmdData.getString("command");

    if(command == SCAN){    //扫描命令
        return LightScan().getBinaryString();

    }else if(command == SCANEND){   //扫描结束命令
        return LightScanEnd().getBinaryString();

    }else if(command == CONNECT){   //连接命令
        string deviceSn = cmdData.getString("deviceSn");
        return LightConnect(deviceSn).getBinaryString();

    }else if(command == ASSIGN_GATEWAY_ADDRESS){    //分配网关地址
        return LightGatewayAddressAssign().getBinaryString();

    }else if(command == ASSIGN_NODE_ADDRESS){   //分配节点地址
        string nodeAddress = cmdData.getString("nodeAddress");
        return LightNodeAddressAssign(nodeAddress).getBinaryString();

    }else if(command == BIND){      //绑定
        return LightBind().getBinaryString();

    }else if(command == UNBIND){    //解绑
        string deviceSn = cmdData.getString("deviceSn");
        return LightUnBind(deviceSn).getBinaryString();

    }else if(command == AddDevice2Group){     //分组
        string deviceSn = cmdData.getString("deviceSn");
        string group_id = cmdData.getString("group_id");
        string model_name = cmdData.getString("model_name");
        return LightAdd2Group(deviceSn, group_id, model_name).getBinaryString();

    }else if(command == DelDeviceFromGroup){    //解除分组
        string deviceSn = cmdData.getString("deviceSn");
        string group_id = cmdData.getString("group_id");
        string model_name = cmdData.getString("model_name");
        return LightDelFromGroup(deviceSn, group_id, model_name).getBinaryString();

    }else if(command == POWER){     //开关
        return LightOnOff(cmdData).getBinaryString();

    }else if(command == LUMINANCE){     //亮度
        return LightLuminance(cmdData).getBinaryString();

    }else if(command == COLORTEMPERATURE){  //色温
        return LightColorTem(cmdData).getBinaryString();
    }

    return string();
}


