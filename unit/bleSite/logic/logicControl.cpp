//
// Created by 78472 on 2022/7/11.
//

#include "logicControl.h"
#include "../parameter.h"
#include "log/Logging.h"
#include "formatTrans/JsonCmd2Binary.h"
#include "formatTrans/lightControlCmd.h"
#include "formatTrans/lightUpStatus.h"
#include <sstream>
#include <iomanip>

static string AssignGateWayAddressString = R"({"command":"assignGateWayAddress"})";
static string AssignNodeAddressString = R"({"command":"assignNodeAddress", "nodeAddress":"0200"})";
static string BindString = R"({"command":"bind"})";

static std::mutex sendMutex;
//向串口发送数据
bool serialSend(unsigned char *buf, int size){
    shared_ptr<BLETelinkDongle> serial = bleConfig::getInstance()->getSerial();
    if(serial != nullptr){
        std::lock_guard<std::mutex> lg(sendMutex);
        if(serial->sendData(buf, static_cast<int>(size))){
            LOG_RED << "===>send success....";
            return true;
        }
    }
    return false;
}

//转换并发送命令
bool transAndSendCmd(qlibc::QData& controlData){
    unsigned char buf[100]{};
    size_t size = bleJsonCmd2Binaray(controlData, buf, 100);
    stringstream ss;
    for(int i = 0; i < size; i++){
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(buf[i]);
        if(i < size -1)
            ss << " ";
    }
    LOG_RED << "==>sendCmd: " << ss.str();
    return serialSend(buf, static_cast<int>(size));
}

atomic<bool> LogicControl::scanFlag{false};

bool LogicControl::parse(qlibc::QData &cmdData) {

    string pseudoCommand  = cmdData.getString("command");
    if(pseudoCommand == SCAN){
        LOG_RED << "start to scan......";
        transAndSendCmd(cmdData);

    }else if(pseudoCommand == CONNECT){
        //连接指令
        LOG_RED << "start to connect......";
        transAndSendCmd(cmdData);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //网关分配地址
        LOG_RED << "start to assign gateway address....";
        qlibc::QData gateAddressAssign(AssignGateWayAddressString);
        transAndSendCmd(gateAddressAssign);
        std::this_thread::sleep_for(std::chrono::seconds(1));

        //节点分配地址
        LOG_RED << "start to assgin node address....";
        qlibc::QData nodeAddressAssign(AssignNodeAddressString);
        transAndSendCmd(nodeAddressAssign);
        EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(20);

        //绑定
        LOG_RED << "start to bind....";
        qlibc::QData bind(BindString);
        transAndSendCmd(bind);
        EventTable::getInstance()->bindSuccessEvent.wait(20);
        LOG_YELLOW << "===>BIND END..........";
    }else{
        transAndSendCmd(cmdData);
    }

    return true;
}
