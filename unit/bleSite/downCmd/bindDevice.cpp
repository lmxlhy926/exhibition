//
// Created by 78472 on 2022/7/19.
//

#include "bindDevice.h"
#include "downCmd/downUtil.h"
#include "upStatus/statusEvent.h"
#include "log/Logging.h"
#include "siteService/service_site_manager.h"
#include "common/httpUtil.h"
#include "sourceManage/scanListmanage.h"
#include "../sourceManage/util.h"

using namespace servicesite;

static const string NetVerifyString = R"({"command": "netVerify"})";
static const string AssignGateWayAddressString = R"({"command":"assignGateWayAddress"})";
static const string BindString = R"({"command":"bind"})";
static const string ScanEndString = R"({"command":"scanEnd"})";

void BindDevice::bind(QData &deviceArray) {
    std::lock_guard<std::mutex> lg(mutex_);
    Json::ArrayIndex arraySize = deviceArray.size();
    //没有待绑定设备，则返回
    if(deviceArray.type() != Json::arrayValue){
        LOG_RED << "===<BIND>: NO DEVICE TO ADD, SCAN END.....";
        qlibc::QData scanEndData(ScanEndString);
        DownUtility::parse2Send(scanEndData);
        return;
    }
    //如果网关已分配地址则跳过，否则给网关分配地址
    if(arraySize > 0){
        LOG_INFO << "===<BIND>: start to verify netKey....";
        qlibc::QData netVerifyCommand(NetVerifyString);
        DownUtility::parse2Send(netVerifyCommand);
        if(EventTable::getInstance()->gateWayNetInfoEvent.wait(10) == std::cv_status::no_timeout){
            qlibc::QData netResData = EventTable::getInstance()->gateWayNetInfoEvent.getData();
            string netKey = netResData.getString("netKey");
            if(!netKey.empty()){
                LOG_PURPLE << "===<BIND>: netKey is already set, <" << netKey << ">......";
                bleConfig::getInstance()->storeNetKey(netKey);
            }else{
                LOG_INFO << "===<BIND>: start to assign gateway address....";
                qlibc::QData gateAddressAssign(AssignGateWayAddressString);
                DownUtility::parse2Send(gateAddressAssign);
                if(EventTable::getInstance()->gateWayIndexEvent.wait(10) == std::cv_status::no_timeout){
                    LOG_PURPLE << "===<BIND>: successed to assgin GatewatAddress， <" << bleConfig::getInstance()->getNetKey() << ">....";
                }else{
                    LOG_RED << "===<BIND>: FAILED TO GatewatAddress, SCAN END....";
                    qlibc::QData scanEndData(ScanEndString);
                    DownUtility::parse2Send(scanEndData);
                    return;
                }
                std::this_thread::sleep_for(std::chrono::seconds(3));

            }
        }else{
            qlibc::QData scanEndData(ScanEndString);
            DownUtility::parse2Send(scanEndData);
            LOG_RED << "===<BIND>: no response from netKey verfify.....";
            return;
        }
    }
    //逐个绑定待绑定设备
    int failedNum = 0, successedNum = 0;
    for(Json::ArrayIndex i = 0; i < arraySize; i++){
        qlibc::QData deviceItemProperty = deviceArray.getArrayElement(i);
        string deviceSn = deviceItemProperty.getString("deviceSn");
        if(addDevice(deviceSn, deviceItemProperty)){
            successedNum++;
            LOG_PURPLE << "<<: " << "seqNum:" << i + 1 << ", successed: " << successedNum << "/" << arraySize << ", "
                       << "failed: " << failedNum << "/" << arraySize;
        }else{
            failedNum++;
            LOG_RED << "<<: " << "seqNum:" << i + 1 << ", successed: " << successedNum << "/" << arraySize << ", "
                    << "failed: " << failedNum << "/" << arraySize;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    //绑定结束，发送停止扫描指令
    LOG_PURPLE << "===<BIND>: ...BIND DEVICE END, SCAN END....";
    qlibc::QData scanEndData(ScanEndString);
    DownUtility::parse2Send(scanEndData);

    //通知设备管理站点更新设备
    util::updateDeviceList();

    //发布绑定结束消息
    qlibc::QData publishData;
    publishData.setString("message_id", BindEndMsg);
    publishData.putData("content", qlibc::QData());
    ServiceSiteManager::getInstance()->publishMessage(BindEndMsg, publishData.toJsonString());
}


bool BindDevice::addDevice(string& deviceSn, qlibc::QData& property) {
    //发送扫描指令
    LOG_INFO << "===<BIND>: scan........";
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    DownUtility::parse2Send(scanData);
    std::this_thread::sleep_for(std::chrono::seconds(3));

//    time_t time_current = time(nullptr);
//    qlibc::QData retScanData;
//    while(retScanData.getString("deviceSn") != deviceSn){
//        if(EventTable::getInstance()->scanResultEvent.wait(2) == std::cv_status::no_timeout){
//            retScanData = EventTable::getInstance()->scanResultEvent.getData();
//        }
//        if(time(nullptr) - time_current > 15){
//            LOG_RED << "<<: FAILED TO SCAN THE DEVICE, QUIT TO ADD THE DEVICE <" << deviceSn << ">";
//            return false;
//        }
//    }
//    LOG_PURPLE << "<<: successed in scaning the device: <" << deviceSn << ">.......";


    //连接设备
    LOG_INFO << "===<BIND>: connect to the device: <" << deviceSn << ">....";
    qlibc::QData connectData;
    connectData.setString("command", "connect");
    connectData.setString("deviceSn", deviceSn);
    DownUtility::parse2Send(connectData);
    std::this_thread::sleep_for(std::chrono::seconds(2));


    //给节点分配地址，大约6秒，等待返回成功，最多等待20秒。
    LOG_INFO << "===<BIND>: start to assgin node address....";
    qlibc::QData nodeAddressAssign(SnAddressMap::getInstance()->getNodeAssignAddr(deviceSn));
    DownUtility::parse2Send(nodeAddressAssign);
    if(EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(20) == std::cv_status::no_timeout){
        LOG_PURPLE << "===<BIND>: successed to assgin node address....";
    }else{
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
        LOG_RED << "===<BIND>: FAILED TO ASSIGN NODE ADDRESS ....";
        return false;
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));


    //绑定；大约需要10秒
    LOG_INFO << "===<BIND>: start to bind....";
    qlibc::QData bind(BindString);
    DownUtility::parse2Send(bind);

    if(EventTable::getInstance()->bindSuccessEvent.wait(30) == std::cv_status::timeout){
        LOG_RED << "<<: xxxxxxxxxBIND FAILEDxxxxxxxx";
        LOG_RED << "<<: xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
        //将绑定失败设备加入扫描列表
        ScanListmanage::getInstance()->appendDeviceItem(deviceSn, property.asValue());
        return false;

    }else{
        qlibc::QData data = EventTable::getInstance()->bindSuccessEvent.getData();
        if(data.getBool("bind")){
            LOG_PURPLE << "<<: ......BIND SUCCESSFULLY..... ";
            LOG_PURPLE << "<<: .............................";

            //将绑定成功设备存入列表中
            bleConfig::getInstance()->insertDeviceItem(deviceSn, property);
            //存入设备的默认状态
            bleConfig::getInstance()->insertDefaultStatus(deviceSn);
            //删除扫描列表中的相应设备
            ScanListmanage::getInstance()->deleteDeviceItem(deviceSn);

            //发布单个设备绑定成功消息
            qlibc::QData content, publishData;
            content.setString("device_id", deviceSn);
            content.setString("device_type", property.getString("device_type"));
            content.setString("device_model", property.getString("device_model"));
            publishData.setString("message_id", SingleDeviceBindSuccessMsg);
            publishData.putData("content", content);
            ServiceSiteManager::getInstance()->publishMessage(SingleDeviceBindSuccessMsg, publishData.toJsonString());
            return true;
        }else{
            LOG_RED << "<<: xxxxxxxxxBIND FAILEDxxxxxxxx";
            LOG_RED << "<<: xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
            SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
            //将绑定失败设备加入扫描列表
            ScanListmanage::getInstance()->appendDeviceItem(deviceSn, property.asValue());
            return false;
        }
    }
}



