//
// Created by 78472 on 2022/7/11.
//

#include "logicControl.h"
#include "siteService/service_site_manager.h"
#include "../parameter.h"
#include "log/Logging.h"
#include "formatTrans/downBinaryUtil.h"
#include "formatTrans/downBinaryCmd.h"
#include "formatTrans/statusEvent.h"
#include "formatTrans/downBinaryUtil.h"
#include "bindDevice.h"
#include <sstream>
#include <iomanip>

using namespace servicesite;

bool LogicControl::parse(qlibc::QData &cmdData) {
    if(bindingFlag.load())  return false;
    string command  = cmdData.getString("command");

    if(command == BIND){      //设备批量绑定
        bindingFlag.store(true);
        qlibc::QData deviceSnArray = cmdData.getData("device_list");
        if(deviceSnArray.size() == 0){
            getScanedDevices(deviceSnArray);
        }
        bd.bind(deviceSnArray);
        bindingFlag.store(false);

    }else if(command == UNBIND){  //设备批量解绑
        qlibc::QData deviceArray = cmdData.getData("device_list");
        unsigned int size = deviceArray.size();
        for(unsigned int i = 0; i < size; ++i){
            qlibc::QData unbindData;
            unbindData.setString("command", "unbind");
            unbindData.setString("deviceSn", deviceArray.getArrayElement(i).asValue().asString());
            DownBinaryCmd::transAndSendCmd(unbindData);
        }

    }else if(command == GROUP){
        string group_name = cmdData.getString("group_name");
        qlibc::QData device_list = cmdData.getData("device_list");
        size_t deviceListSize = device_list.size();
        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            qlibc::QData groupData;
            groupData.setString("command", "group");
            groupData.setString("groupName", group_name);
            groupData.setString("deviceSn", device_list.getArrayElement(i).asValue().asString());
            DownBinaryCmd::transAndSendCmd(groupData);
        }

    }else{  //设备控制指令
        DownBinaryCmd::transAndSendCmd(cmdData);
    }

    return true;
}


//进行指定时间的扫描，获取可连接的设备列表
void LogicControl::getScanedDevices(qlibc::QData& deviceArray){
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    DownBinaryCmd::transAndSendCmd(scanData);

    std::map<string, int> deviceMap;
    qlibc::QData retScanData;
    time_t time_current = time(nullptr);

    while(true){
        if(EventTable::getInstance()->scanResultEvent.wait(2) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
            string deviceSn = retScanData.getString("deviceSn");
            if(deviceSn.substr(0, 2) != "00"){
                deviceMap.insert(std::make_pair(deviceSn, 0));
            }
        }
        if(time(nullptr) - time_current > 10){
            LOG_PURPLE << "===>SCAN END......";
            break;
        }
    }

    for(auto& elem : deviceMap){
        deviceArray.append(Json::Value(elem.first));
    }

    LOG_HLIGHT << "==>deviceArray: " << deviceArray.toJsonString(true);

    //发布扫描结果
    qlibc::QData content, publishData;
    content.putData("device_list", deviceArray);
    publishData.setString("message_id", ScanResultMsg);
    publishData.putData("content", content);
    ServiceSiteManager::getInstance()->publishMessage(ScanResultMsg, publishData.toJsonString());
}

