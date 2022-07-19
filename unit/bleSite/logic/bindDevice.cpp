//
// Created by 78472 on 2022/7/19.
//

#include "bindDevice.h"
#include "formatTrans/downBinaryCmd.h"
#include "formatTrans/statusEvent.h"
#include "log/Logging.h"

static string AssignGateWayAddressString = R"({"command":"assignGateWayAddress"})";
static string AssignNodeAddressString1 = R"({"command":"assignNodeAddress", "nodeAddress":"0201"})";
static string AssignNodeAddressString2 = R"({"command":"assignNodeAddress", "nodeAddress":"0202"})";
static string BindString = R"({"command":"bind"})";


bool addDevice(string& deviceSn, Json::ArrayIndex index) {
    //0. 扫描


    qlibc::QData connectData;
    connectData.setString("command", "connect");
    connectData.setString("deviceSn", deviceSn);

    //1. 连接
    DownBinaryCmd::transAndSendCmd(connectData);
    std::this_thread::sleep_for(std::chrono::seconds(1));

    if(index == 0){
        //2. 网关分配地址
        LOG_YELLOW << "start to assign gateway address....";
        qlibc::QData gateAddressAssign(AssignGateWayAddressString);
        DownBinaryCmd::transAndSendCmd(gateAddressAssign);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


    //3. 节点分配地址
    LOG_YELLOW << "start to assgin node address....";
    if(index == 0){
        qlibc::QData nodeAddressAssign(AssignNodeAddressString1);
        DownBinaryCmd::transAndSendCmd(nodeAddressAssign);
        EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(20);

    }else if(index == 1){
        qlibc::QData nodeAddressAssign(AssignNodeAddressString2);
        DownBinaryCmd::transAndSendCmd(nodeAddressAssign);
        EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(20);
    }

    //4. 绑定
    LOG_YELLOW << "start to bind....";
    qlibc::QData bind(BindString);
    DownBinaryCmd::transAndSendCmd(bind);
    EventTable::getInstance()->bindSuccessEvent.wait(20);

    LOG_RED << "===>BIND END..........";

    return true;
}



void BindDevice::operator()() {
    Json::ArrayIndex arraySize = deviceArray_.size();
    if(deviceArray_.type() != Json::arrayValue) return;
    for(Json::ArrayIndex i = 0; i < arraySize; i++){
        string deviceSn = deviceArray_.getArrayElement(i).asValue().asString();
        LOG_RED << "==>deviceSn: " << deviceSn;
        addDevice(deviceSn, i);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

