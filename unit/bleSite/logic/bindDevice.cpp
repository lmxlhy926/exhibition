//
// Created by 78472 on 2022/7/19.
//

#include "bindDevice.h"
#include "formatTrans/downBinaryCmd.h"
#include "formatTrans/statusEvent.h"
#include "log/Logging.h"
#include "../parameter.h"
#include "siteService/service_site_manager.h"
#include "common/httpUtil.h"

using namespace servicesite;

static string AssignGateWayAddressString = R"({"command":"assignGateWayAddress"})";
static string BindString = R"({"command":"bind"})";


void BindDevice::bind(QData &deviceArray) {
    std::lock_guard<std::mutex> lg(mutex_);
    Json::ArrayIndex arraySize = deviceArray.size();
    if(deviceArray.type() != Json::arrayValue) return;
    for(Json::ArrayIndex i = 0; i < arraySize; i++){
        string deviceSn = deviceArray.getArrayElement(i).getString("device_id");
        string device_type = deviceArray.getArrayElement(i).getString("device_type");
        string device_model = deviceArray.getArrayElement(i).getString("device_model");
        addDevice(deviceSn, device_type, device_model);
        if(i != arraySize - 1 ){
            std::this_thread::sleep_for(std::chrono::seconds(3));
        }
    }

    //更新配置站点的白名单
    updateDeviceList2ConfigSite();

    //发布绑定结束消息
    qlibc::QData publishData;
    publishData.setString("message_id", BindEndMsg);
    publishData.putData("content", qlibc::QData());
    ServiceSiteManager::getInstance()->publishMessage(BindEndMsg, publishData.toJsonString());
}

bool BindDevice::addDevice(string &deviceSn, string& device_type, string& device_model) {
    //发送扫描指令
    LOG_INFO << ">>: start to scan the device <" << deviceSn << ">.....";
    qlibc::QData scanData;
    scanData.setString("command", "scan");
    DownBinaryCmd::transAndSendCmd(scanData);

    //等待扫描到指定的设备
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

    //发送设备连接指令，等待1秒
    LOG_INFO << ">>: start to connect the device: <" << deviceSn << ">....";
    qlibc::QData connectData;
    connectData.setString("command", "connect");
    connectData.setString("deviceSn", deviceSn);
    DownBinaryCmd::transAndSendCmd(connectData);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LOG_PURPLE << "<<: successed in connecting the device: <" << deviceSn << ">....";

    //给网关分配地址，等待1秒
    LOG_INFO << ">>: start to assign gateway address....";
    qlibc::QData gateAddressAssign(AssignGateWayAddressString);
    DownBinaryCmd::transAndSendCmd(gateAddressAssign);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    LOG_PURPLE << "<<: successed to assign gateway address....";

    //给节点分配地址，等待返回成功，最多等待60秒
    LOG_INFO << ">>: start to assgin node address....";
    qlibc::QData nodeAddressAssign(SnAddressMap::getInstance()->getNodeAssignAddr(deviceSn));
    DownBinaryCmd::transAndSendCmd(nodeAddressAssign);
    if(EventTable::getInstance()->nodeAddressAssignSuccessEvent.wait(60) == std::cv_status::no_timeout){
        LOG_PURPLE << "<<: successed to assgin node address....";
    }else{
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
        LOG_RED << "<<: FAILED TO ASSIGN NODE ADDRESS....";
        return false;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));

    //绑定；大约需要40秒
    LOG_INFO << ">>: start to bind....";
    qlibc::QData bind(BindString);
    DownBinaryCmd::transAndSendCmd(bind);

    if(EventTable::getInstance()->bindSuccessEvent.wait(60) == std::cv_status::timeout){
        LOG_RED << "<<: .......BIND FAILED..........";
        LOG_RED << "<<: ----------------------------";
        LOG_RED << "<<: -----------------------------";
        SnAddressMap::getInstance()->deleteDeviceSn(deviceSn);
        return false;
    }

    LOG_PURPLE << "<<: ......BIND SUCCESSFULLY..... ";
    LOG_PURPLE << "<<: .............................";
    LOG_PURPLE << "<<: .............................";

    //将绑定成功设备存入列表中
    bleConfig::getInstance()->insertDeviceItem(deviceSn, device_type, device_model);
    //存入设备的默认状态
    bleConfig::getInstance()->insertDefaultStatus(deviceSn);

    //发布单个设备绑定成功消息
    qlibc::QData content, publishData;
    content.setString("device_id", deviceSn);
    content.setString("device_type", device_type);
    content.setString("device_model", device_model);
    publishData.setString("message_id", SingleDeviceBindSuccessMsg);
    publishData.putData("content", content);
    ServiceSiteManager::getInstance()->publishMessage(SingleDeviceBindSuccessMsg, publishData.toJsonString());

    return true;
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


