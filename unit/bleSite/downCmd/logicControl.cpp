//
// Created by 78472 on 2022/7/11.
//

#include "logicControl.h"
#include "siteService/service_site_manager.h"
#include "../parameter.h"
#include "log/Logging.h"
#include "formatTrans/downUtil.h"
#include "formatTrans/statusEvent.h"
#include "logic/scanListmanage.h"
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
        getScanedDevices(deviceSnArray, param);
        bd.bind(deviceSnArray);
        bindingFlag.store(false);

    }else{  //设备控制指令
        DownUtility::parse2Send(cmdData);
    }

    return true;
}


//进行指定时间的扫描，获取可连接的设备列表
void LogicControl::getScanedDevices(qlibc::QData& deviceArray, qlibc::QData& param){
    LOG_INFO << "start to find device.....";
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    DownUtility::parse2Send(scanData);

    std::map<string, Json::Value> deviceMap;
    qlibc::QData retScanData;
    time_t time_current = time(nullptr);

    while(true){
        if(EventTable::getInstance()->scanResultEvent.wait(2) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
            string deviceSn = retScanData.getString("deviceSn");
            if(!deviceSn.empty()){
                retScanData.setString("room_name", param.getString("room_name"));
                retScanData.setString("room_no", param.getString("room_no"));
                deviceMap.insert(std::make_pair(deviceSn, retScanData.asValue()));
            }
        }
        if(time(nullptr) - time_current > 10){
            LOG_PURPLE << "===>find device end....";
            break;
        }
    }

    //存储到扫描列表
    for(auto& elem : deviceMap){
        ScanListmanage::getInstance()->appendDeviceItem(elem.first, elem.second);
    }
    std::map<string, Json::Value> scanedMap;
    bool bindFailedDevice = param.getBool("bindFailedDevice");
    if(bindFailedDevice){
        scanedMap = ScanListmanage::getInstance()->getScanListMap();
    }else{
        scanedMap = deviceMap;
    }

    qlibc::QData scanedArray;
    for(auto& elem : scanedMap){
        deviceArray.append(elem.second);

        qlibc::QData item;
        item.setString("device_id", elem.first);
        item.setString("device_type", elem.second["device_type"].asString());
        item.setString("device_model", elem.second["device_model"].asString());
        scanedArray.append(item);
    }

    LOG_GREEN << "==>deviceArray: " << deviceArray.toJsonString(true);

    //发布扫描结果
    qlibc::QData content, publishData;
    content.putData("device_list", scanedArray);
    publishData.setString("message_id", ScanResultMsg);
    publishData.putData("content", content);
    ServiceSiteManager::getInstance()->publishMessage(ScanResultMsg, publishData.toJsonString());
}

