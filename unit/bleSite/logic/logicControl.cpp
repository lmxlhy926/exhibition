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
        //连接指令
        LOG_BLUE << "start to connect......";
        DownBinaryCmd::transAndSendCmd(cmdData);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //网关分配地址
        LOG_BLUE << "start to assign gateway address....";
        qlibc::QData gateAddressAssign(AssignGateWayAddressString);
        DownBinaryCmd::transAndSendCmd(gateAddressAssign);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //节点分配地址
        LOG_BLUE << "start to assgin node address....";
        qlibc::QData nodeAddressAssign(AssignNodeAddressString);
        DownBinaryCmd::transAndSendCmd(nodeAddressAssign);
        EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(20);

        //绑定
        LOG_BLUE << "start to bind....";
        qlibc::QData bind(BindString);
        DownBinaryCmd::transAndSendCmd(bind);
        EventTable::getInstance()->bindSuccessEvent.wait(20);

        LOG_PURPLE << "===>BIND END..........";
    }else{
        DownBinaryCmd::transAndSendCmd(cmdData);
    }

    return true;
}
