//
// Created by 78472 on 2022/6/2.
//

#include "contreteDeviceControl.h"
#include "../paramConfig.h"

void CommonControl::operator()(const DownCommandData &downCommand, qlibc::QData &deviceList) {
    for(int i = 0 ; i < deviceList.size(); i++){
        qlibc::QData ithData = deviceList.getArrayElement(i);

        if(match(downCommand, ithData)){
            //构造控制命令
            qlibc::QData controlCommand = constructCtrCmd(downCommand, ithData);

            //发送控制请求
            qlibc::QData controlRet;
            ControlBase::sitePostRequest(AdapterIp, AdapterPort, controlCommand, controlRet);
            std::cout << "===>controlCommand to Adapter: " << controlCommand.toJsonString(true) << std::endl;

            break;
        }

        if(i == deviceList.size() - 1)
            std::cout << "===>no matched deviceItem" << std::endl;
    }
}

bool CommonControl::match(const DownCommandData &downCommand, qlibc::QData &deviceItem) {
    return ControlBase::match(downCommand, deviceItem);
}

qlibc::QData CommonControl::constructCtrCmd(const DownCommandData &downCommand, qlibc::QData &deviceItem) {
    return ControlBase::constructCtrCmd(downCommand, deviceItem);
}


