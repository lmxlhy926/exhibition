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

    if(command == ADD_DEVICE){      //设备批量绑定
        bindingFlag.store(true);

        qlibc::QData param = cmdData.getData("param");
        qlibc::QData deviceSnArray;
        getScanedDevices(deviceSnArray);

        bd.bind(deviceSnArray);
        bindingFlag.store(false);

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

    std::map<string, std::pair<string, string>> deviceMap;
    qlibc::QData retScanData;
    time_t time_current = time(nullptr);

    while(true){
        if(EventTable::getInstance()->scanResultEvent.wait(2) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
            string deviceSn = retScanData.getString("deviceSn");
            string device_type = retScanData.getString("device_type");
            string device_model = retScanData.getString("device_model");
            deviceMap.insert(std::make_pair(deviceSn, std::make_pair(device_type, device_model)));
        }
        if(time(nullptr) - time_current > 10){
            LOG_PURPLE << "===>SCAN END......";
            break;
        }
    }


    for(auto& elem : deviceMap){
        qlibc::QData item;
        item.setString("device_id", elem.first);
        item.setString("device_type", elem.second.first);
        item.setString("device_model", elem.second.second);
        deviceArray.append(item);
    }

    LOG_GREEN << "==>deviceArray: " << deviceArray.toJsonString(true);

    //发布扫描结果
    qlibc::QData content, publishData;
    content.putData("device_list", deviceArray);
    publishData.setString("message_id", ScanResultMsg);
    publishData.putData("content", content);
    ServiceSiteManager::getInstance()->publishMessage(ScanResultMsg, publishData.toJsonString());
}

