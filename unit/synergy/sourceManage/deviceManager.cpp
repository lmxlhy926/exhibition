//
// Created by 78472 on 2022/9/26.
//

#include "deviceManager.h"
#include "common/httpUtil.h"
#include "../param.h"

DeviceManager* DeviceManager::instance = nullptr;

DeviceManager *DeviceManager::getInstance() {
    if(instance == nullptr){
        instance = new DeviceManager();
    }
    return instance;
}

void DeviceManager::listChanged() {
    changed.store(true);
}


qlibc::QData DeviceManager::getAllDeviceList() {
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    return deviceList_;
}

bool DeviceManager::isInDeviceList(string& device_id, string& inSourceSite, string& outSourceSite){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    Json::ArrayIndex size = deviceList_.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList_.getArrayElement(i);
        string itemDeviceUid = item.getString("device_uid");
        string deviceUid = device_id;
        if(!inSourceSite.empty()){
            deviceUid.append(">").append(inSourceSite);
        }

        if(itemDeviceUid == deviceUid){
            smatch sm;
            if(regex_match(itemDeviceUid, sm, regex("(.*)>(.*)"))){
                outSourceSite = sm.str(2);
                return true;
            }
        }
    }
    return false;
}

qlibc::QData DeviceManager::restoreMac(qlibc::QData& item, string& inSourceSite){
    if(inSourceSite.empty()){
        string device_id = item.getString("device_id");
        smatch sm;
        if(regex_match(device_id, sm, regex("(.*)>(.*)"))){
            device_id = sm.str(1);
            return item.setString("device_id", device_id);
        }
    }
    item.removeMember("sourceSite");
    return item;
}

void DeviceManager::updateDeviceList(){
    qlibc::QData totalList;     //存储总列表
    std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
    smatch sm;
    for(auto& elem : siteNameSet){
        if(regex_match(elem, sm, regex("(.*):(.*)"))){
            string uuid = sm.str(1);
            string siteID = sm.str(2);
            if((siteID == BleSiteID || siteID == TvAdapterSiteID || siteID == ZigbeeSiteID)){
                qlibc::QData deviceRequest;
                deviceRequest.setString("service_id", "get_device_list");
                deviceRequest.setValue("request", Json::nullValue);
                qlibc::QData deviceRes;
                SiteRecord::getInstance()->sendRequest2Site(sm.str(0), deviceRequest, deviceRes);       //获取设备列表
                qlibc::QData list = addMacSource(deviceRes.getData("response").getData("device_list"),
                                                 string().append(uuid).append(":").append(siteID));     //给列表条目加入来源标签
                mergeList(list, totalList);
            }
        }
    }
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    deviceList_ = totalList;
}


qlibc::QData DeviceManager::addMacSource(qlibc::QData deviceList, string sourceTag){
    Json::ArrayIndex num = deviceList.size();
    qlibc::QData newDeviceList;
    for(Json::ArrayIndex i = 0; i < num; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string device_id = item.getString("device_id");
        device_id.append(">").append(sourceTag);
        item.setString("sourceSite", sourceTag);    //标记设备来源
        item.setString("device_uid", device_id);    //device_uid是唯一的
        newDeviceList.append(item);
    }
    return newDeviceList;
}


void DeviceManager::mergeList(qlibc::QData &list, qlibc::QData &totalList) {
    for(Json::ArrayIndex i = 0; i < list.size(); ++i){
        qlibc::QData item = list.getArrayElement(i);
        totalList.append(item);
    }
}





