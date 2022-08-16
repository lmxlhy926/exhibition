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

    if(pseudoCommand == BIND){
        bindingFlag.store(true);
        qlibc::QData deviceSnArray = cmdData.getData("deviceSn");
        if(deviceSnArray.size() == 0){
            getScanedDevices(deviceSnArray);
        }
        bd.bind(deviceSnArray);
        bindingFlag.store(false);

    }else if(pseudoCommand == UNBIND){
        qlibc::QData deviceArray = cmdData.getData("deviceSn");
        unsigned int size = deviceArray.size();
        for(unsigned int i = 0; i < size; ++i){
            qlibc::QData unbindData;
            unbindData.setString("command", "unbind");
            unbindData.setString("deviceSn", deviceArray.getArrayElement(i).asValue().asString());
            DownBinaryCmd::transAndSendCmd(unbindData);
            this_thread::sleep_for(std::chrono::seconds(1));
        }
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
        if(EventTable::getInstance()->scanResultEvent.wait(2) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
            string deviceSn = retScanData.getString("deviceSn");
            deviceMap.insert(std::make_pair(deviceSn, 0));
        }
        if(time(nullptr) - time_current > 15){
            LOG_PURPLE << "===>SCAN END, start to bind devices......";
            break;
        }
    }

    for(auto& elem : deviceMap){
        deviceArray.append(Json::Value(elem.first));
    }

    LOG_HLIGHT << "==>deviceArray: " << deviceArray.toJsonString(true);
}

