//
// Created by 78472 on 2022/7/11.
//

#include "logicControl.h"
#include "siteService/service_site_manager.h"
#include "../parameter.h"
#include "log/Logging.h"
#include "downCmd/downUtil.h"
#include "upStatus/statusEvent.h"
#include "sourceManage/scanListmanage.h"

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
    LOG_INFO << "===<SCAN>: start to find device.....";
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    DownUtility::parse2Send(scanData);

    //提取允许添加的设备
    bool enableAllowList = param.getBool("enableAllowList");
    qlibc::QData allowAddList = param.getData("allowAddList");
    Json::Value::ArrayIndex allowAddListSize = allowAddList.size();

    //将扫描到的设备存储到设备列表
    std::map<string, Json::Value> deviceMap;
    qlibc::QData retScanData;
    time_t time_current = time(nullptr);
    int timeSeconds = param.getInt("timeSeconds");
    if(timeSeconds <= 0){
        timeSeconds = 10;
    }

    while(true){
        if(EventTable::getInstance()->scanResultEvent.wait(2) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
            string deviceSn = retScanData.getString("deviceSn");
            if(!deviceSn.empty()){
                retScanData.putData("location", param.getData("location"));
                deviceMap.insert(std::make_pair(deviceSn, retScanData.asValue()));
            }
        }
        if(time(nullptr) - time_current > timeSeconds){
            LOG_PURPLE << "===<SCAN>: find device end....";
            break;
        }
    }

    std::map<string, Json::Value> scanedMap;
    bool bindFailedDevice = param.getBool("bindFailedDevice");
    if(bindFailedDevice){   //将失败的设备添加进来
        std::map<string, Json::Value> failedMap = ScanListmanage::getInstance()->getScanListMap();
        scanedMap = deviceMap;
        for(auto& failedItem : failedMap){
            scanedMap.insert(std::make_pair(failedItem.first, failedItem.second));
        }
        LOG_PURPLE << "===<SCAN>: THIS SCAN RESULT + FAILED DEVICE.....";
    }else{
        scanedMap = deviceMap;
        LOG_PURPLE << "===<SCAN>: THIS SCAN RESULT.....";
    }

    //剔除不在允许安装列表中的设备
    if(enableAllowList){
        for(auto pos = scanedMap.begin(); pos != scanedMap.end();){
            bool isFind = false;
            for(Json::ArrayIndex i = 0; i < allowAddListSize; ++i){
                string deviceSn = allowAddList.getArrayElement(i).asValue().asString();
                if(pos->first == deviceSn){
                    pos++;
                    isFind = true;
                    break;
                }
            }
            if(!isFind){
                pos = scanedMap.erase(pos);
            }
        }
    }

    qlibc::QData scanedArray;
    string sourceSite = util::getSourceSite();
    for(auto& elem : scanedMap){
        deviceArray.append(elem.second);

        qlibc::QData item;
        item.setString("device_id", elem.first);
        item.setString("device_type", elem.second["device_type"].asString());
        item.setString("device_model", elem.second["device_model"].asString());
        item.setString("sourceSite", sourceSite);
        scanedArray.append(item);
    }
    LOG_GREEN << "===<SCAN>: deviceArray: " << deviceArray.toJsonString(true);

    //发布扫描结果
    qlibc::QData content, publishData;
    content.putData("device_list", scanedArray);
    publishData.setString("message_id", ScanResultMsg);
    publishData.putData("content", content);
    ServiceSiteManager::getInstance()->publishMessage(ScanResultMsg, publishData.toJsonString());
}

