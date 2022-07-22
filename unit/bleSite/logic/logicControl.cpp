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
        bd.bind(deviceSnArray);
        bindingFlag.store(false);

    }else{
        DownBinaryCmd::transAndSendCmd(cmdData);
    }

    return true;
}
