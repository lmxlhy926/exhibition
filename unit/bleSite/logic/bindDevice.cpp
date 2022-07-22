//
// Created by 78472 on 2022/7/19.
//

#include "bindDevice.h"
#include "formatTrans/downBinaryCmd.h"
#include "formatTrans/statusEvent.h"
#include "log/Logging.h"

static string AssignGateWayAddressString = R"({"command":"assignGateWayAddress"})";
static string BindString = R"({"command":"bind"})";

void BindDevice::bind(QData &deviceArray) {
    Json::ArrayIndex arraySize = deviceArray.size();
    if(deviceArray.type() != Json::arrayValue) return;
    for(Json::ArrayIndex i = 0; i < arraySize; i++){
        string deviceSn = deviceArray.getArrayElement(i).asValue().asString();
        addDevice(deviceSn, i);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
}

bool BindDevice::addDevice(string &deviceSn, Json::ArrayIndex index) {
    //0. 扫描
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    LOG_YELLOW << ">>: start to add device: <" << deviceSn << ">.........";
    LOG_YELLOW << ">>: start to scan the device <" << deviceSn << ">.....";
    DownBinaryCmd::transAndSendCmd(scanData);
    qlibc::QData retScanData;
    while(retScanData.getString("deviceSn") != deviceSn){
        if(EventTable::getInstance()->scanResultEvent.wait(20) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
        }else{
            LOG_RED << "<<: FAILED TO SCAN THE DEVICE, QUIT TO ADD THE DEVICE <" << deviceSn << ">";
            return false;
        }
    }
    LOG_RED << "<<: successed in scaning the device: <" << deviceSn << ">.......";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //1. 连接
    LOG_YELLOW << ">>: start to connect the device: <" << deviceSn << ">....";
    qlibc::QData connectData;
    connectData.setString("command", "connect");
    connectData.setString("deviceSn", deviceSn);
    DownBinaryCmd::transAndSendCmd(connectData);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LOG_RED << "<<: successed in connecting the device: <" << deviceSn << ">....";

//    if(index == 0){
    //2. 网关分配地址
    LOG_YELLOW << ">>: start to assign gateway address....";
    qlibc::QData gateAddressAssign(AssignGateWayAddressString);
    DownBinaryCmd::transAndSendCmd(gateAddressAssign);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LOG_RED << "<<: successed to assign gateway address....";
//    }

    //3. 节点分配地址
    LOG_YELLOW << ">>: start to assgin node address....";
    qlibc::QData nodeAddressAssign(snAddrMap.getNodeAssignAddr(deviceSn));
    DownBinaryCmd::transAndSendCmd(nodeAddressAssign);
    if(EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(30) == std::cv_status::no_timeout){
        LOG_RED << "<<: successed to assgin node address....";
    }else{
        snAddrMap.deleteDeviceSn(deviceSn);
        LOG_RED << "<<: FAILED TO ASSIGN NODE ADDRESS....";
        return false;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //4. 绑定
    LOG_YELLOW << ">>: start to bind....";
    qlibc::QData bind(BindString);
    DownBinaryCmd::transAndSendCmd(bind);

    if(EventTable::getInstance()->bindSuccessEvent.wait(20) == std::cv_status::timeout){
        LOG_RED << "<<: ..........BIND FAILED..........";
        LOG_RED << "<<: -------------------------------";
        LOG_RED << "<<: -------------------------------";
        LOG_RED << "<<: -------------------------------";
        return false;
    }
    LOG_RED << "<<: ..........BIND END SUCCESSFULLY..........";
    LOG_RED << "<<: .............................";
    LOG_RED << "<<: .............................";
    LOG_RED << "<<: .............................";

    std::this_thread::sleep_for(std::chrono::seconds(3));

    return true;
}

