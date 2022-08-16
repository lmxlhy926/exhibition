//
// Created by 78472 on 2022/7/11.
//

#include "logicControl.h"
#include "../parameter.h"
#include "log/Logging.h"
#include "formatTrans/downBinaryUtil.h"
#include "formatTrans/downBinaryCmd.h"
#include "formatTrans/statusEvent.h"
#include "formatTrans/downBinaryUtil.h"
#include "bindDevice.h"
#include <sstream>
#include <iomanip>


bool LogicControl::parse(qlibc::QData &cmdData) {
    if(bindingFlag.load())  return false;

    string pseudoCommand  = cmdData.getString("command");
    if(pseudoCommand == SCAN){
        LOG_YELLOW << "start to scan......";
        DownBinaryCmd::transAndSendCmd(cmdData);

    }else if(pseudoCommand == CONNECT){
        bindingFlag.store(true);
        qlibc::QData deviceSnArray = cmdData.getData("deviceSn");
//        getScanedDevices(deviceSnArray);
        bd.bind(deviceSnArray);
        bindingFlag.store(false);

    }else{
        DownBinaryCmd::transAndSendCmd(cmdData);
    }

    return true;
}

void LogicControl::getScanedDevices(qlibc::QData& deviceArray){
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    DownBinaryCmd::transAndSendCmd(scanData);

    std::map<string, int> deviceMap;
    qlibc::QData retScanData;
    time_t time_current = time(nullptr);

    while(true){
        if(EventTable::getInstance()->scanResultEvent.wait(20) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
            string deviceSn = retScanData.getString("deviceSn");
            deviceMap.insert(std::make_pair(deviceSn, 0));
        }
        if(time(nullptr) - time_current > 10){
            LOG_PURPLE << "===>SCAN END, start to bind devices......";
            break;
        }
    }

    for(auto& elem : deviceMap){
        deviceArray.append(Json::Value(elem.first));
    }

    LOG_HLIGHT << "==>deviceArray: " << deviceArray.toJsonString(true);
}

