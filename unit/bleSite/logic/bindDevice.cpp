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
    LOG_INFO << ">>: start to scan the device <" << deviceSn << ">.....";
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    DownBinaryCmd::transAndSendCmd(scanData);

    time_t time_current = time(nullptr);
    qlibc::QData retScanData;
    while(retScanData.getString("deviceSn") != deviceSn){
        if(EventTable::getInstance()->scanResultEvent.wait(2) == std::cv_status::no_timeout){
            retScanData = EventTable::getInstance()->scanResultEvent.getData();
        }
        if(time(nullptr) - time_current > 20){
            LOG_RED << "<<: FAILED TO SCAN THE DEVICE, QUIT TO ADD THE DEVICE <" << deviceSn << ">";
            return false;
        }
    }
    LOG_PURPLE << "<<: successed in scaning the device: <" << deviceSn << ">.......";
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //1. 连接；大约10秒
    LOG_INFO << ">>: start to connect the device: <" << deviceSn << ">....";
    qlibc::QData connectData;
    connectData.setString("command", "connect");
    connectData.setString("deviceSn", deviceSn);
    DownBinaryCmd::transAndSendCmd(connectData);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LOG_PURPLE << "<<: successed in connecting the device: <" << deviceSn << ">....";

    //2. 网关分配地址
    LOG_INFO << ">>: start to assign gateway address....";
    qlibc::QData gateAddressAssign(AssignGateWayAddressString);
    DownBinaryCmd::transAndSendCmd(gateAddressAssign);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LOG_PURPLE << "<<: successed to assign gateway address....";

    //3. 节点分配地址；大约需要40秒
    LOG_INFO << ">>: start to assgin node address....";
    qlibc::QData nodeAddressAssign(snAddrMapPtr->getNodeAssignAddr(deviceSn));
    DownBinaryCmd::transAndSendCmd(nodeAddressAssign);
    if(EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(60) == std::cv_status::no_timeout){
        LOG_PURPLE << "<<: successed to assgin node address....";
    }else{
        snAddrMapPtr->deleteDeviceSn(deviceSn);
        LOG_RED << "<<: FAILED TO ASSIGN NODE ADDRESS....";
        return false;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //4. 绑定；大约需要40秒
    LOG_INFO << ">>: start to bind....";
    qlibc::QData bind(BindString);
    DownBinaryCmd::transAndSendCmd(bind);

    if(EventTable::getInstance()->bindSuccessEvent.wait(60) == std::cv_status::timeout){
        LOG_RED << "<<: .......BIND FAILED..........";
        LOG_RED << "<<: ----------------------------";
        LOG_RED << "<<: -----------------------------";
        snAddrMapPtr->deleteDeviceSn(deviceSn);
        return false;
    }

    LOG_PURPLE << "<<: ......BIND SUCCESSFULLY..... ";
    LOG_PURPLE << "<<: .............................";
    LOG_PURPLE << "<<: .............................";

    return true;
}

