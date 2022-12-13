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

using namespace servicesite;

static const string AssignGateWayAddressString = R"({"command":"assignGateWayAddress"})";
static const string BindString = R"({"command":"bind"})";
static const string ScanEndString = R"({"command":"scanEnd"})";


void BindDevice::bind(QData &deviceArray) {
    std::lock_guard<std::mutex> lg(mutex_);
    Json::ArrayIndex arraySize = deviceArray.size();
    if(deviceArray.type() != Json::arrayValue){
        LOG_RED << "NO DEVICE TO ADD, scan end.....";
        qlibc::QData scanEndData(ScanEndString);
        DownUtility::parse2Send(scanEndData);
        return;
    }
    if(arraySize > 0){   //给网关分配地址
        LOG_INFO << ">>: start to assign gateway address....";
        qlibc::QData gateAddressAssign(AssignGateWayAddressString);
        DownUtility::parse2Send(gateAddressAssign);
        if(EventTable::getInstance()->gateWayIndexEvent.wait(10) == std::cv_status::no_timeout){
            LOG_PURPLE << "<<: successed to assgin GatewatAddress....";
        }else{
            LOG_RED << "<<: FAILED TO GatewatAddress, scan end....";
            qlibc::QData scanEndData(ScanEndString);
            DownUtility::parse2Send(scanEndData);
            return;
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    for(Json::ArrayIndex i = 0; i < arraySize; i++){
        qlibc::QData deviceItemProperty = deviceArray.getArrayElement(i);
        string deviceSn = deviceItemProperty.getString("deviceSn");
        addDevice(deviceSn, deviceItemProperty);
        LOG_PURPLE << "<<: " << deviceItemProperty.getString("room_name") << "-------" << i + 1 << "/" << arraySize << "-------";
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    LOG_PURPLE << "...BIND DEVICE END, SCAN END....";
    qlibc::QData scanEndData(ScanEndString);
    DownUtility::parse2Send(scanEndData);

    //更新配置站点的白名单
    updateDeviceList2ConfigSite();

    //发布绑定结束消息
    qlibc::QData publishData;
    publishData.setString("message_id", BindEndMsg);
    publishData.putData("content", qlibc::QData());
    ServiceSiteManager::getInstance()->publishMessage(BindEndMsg, publishData.toJsonString());
}

bool BindDevice::addDevice(string& deviceSn, qlibc::QData& property) {
    //扫描到指定设备
    LOG_INFO << ">>: scan........";
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
    LOG_INFO << ">>: connect to the device: <" << deviceSn << ">....";
    qlibc::QData connectData;
    connectData.setString("command", "connect");
    connectData.setString("deviceSn", deviceSn);
    DownUtility::parse2Send(connectData);
    std::this_thread::sleep_for(std::chrono::seconds(2));


    //给节点分配地址，大约6秒，等待返回成功，最多等待20秒。分配失败，依然进行绑定
    LOG_INFO << ">>: start to assgin node address....";
    qlibc::QData nodeAddressAssign(SnAddressMap::getInstance()->getNodeAssignAddr(deviceSn));
    DownUtility::parse2Send(nodeAddressAssign);
    if(EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(20) == std::cv_status::no_timeout){
        LOG_PURPLE << "<<: successed to assgin node address....";
    }else{
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
        LOG_RED << "<<: FAILED TO ASSIGN NODE ADDRESS, start to bind device ....";
    }
    std::this_thread::sleep_for(std::chrono::seconds(2));


    //绑定；大约需要10秒
    LOG_INFO << ">>: start to bind....";
    qlibc::QData bind(BindString);
    DownUtility::parse2Send(bind);

    if(EventTable::getInstance()->bindSuccessEvent.wait(120) == std::cv_status::timeout){
        LOG_RED << "<<: xxxxxxxxxBIND FAILEDxxxxxxxx";
        LOG_RED << "<<: xxxxxxxxxxxxxxxxxxxxxxxxxxxx";
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
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
            return false;
        }
    }
}

void BindDevice::updateDeviceList2ConfigSite() {
    //更新config白名单列表
    qlibc::QData request;
    request.setString("service_id", "whiteListUpdateRequest");
    request.putData("request", bleConfig::getInstance()->getDeviceListData());
    qlibc::QData response;
    SiteRecord::getInstance()->sendRequest2Site(ConfigSiteName, request, response);
    LOG_HLIGHT << "==>updateDeviceList2ConfigSite";
}


