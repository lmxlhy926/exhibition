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


static string AssignGateWayAddressString = R"({"command":"assignGateWayAddress"})";
static string AssignNodeAddressString = R"({"command":"assignNodeAddress", "nodeAddress":"0200"})";
static string BindString = R"({"command":"bind"})";


atomic<bool> LogicControl::scanFlag{false};

bool LogicControl::parse(qlibc::QData &cmdData) {

    string pseudoCommand  = cmdData.getString("command");
    if(pseudoCommand == SCAN){
        LOG_BLUE << "start to scan......";
        DownBinaryCmd::transAndSendCmd(cmdData);

    }else if(pseudoCommand == CONNECT){
        qlibc::QData deviceSnArray = cmdData.getData("deviceSn");
        BindDevice(deviceSnArray).operator()();
    }else{
        DownBinaryCmd::transAndSendCmd(cmdData);
    }

    return true;
}
