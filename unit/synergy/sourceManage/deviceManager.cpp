//
// Created by 78472 on 2022/9/26.
//

#include "deviceManager.h"
#include "common/httpUtil.h"
#include "../param.h"
#include "log/Logging.h"

DeviceManager* DeviceManager::instance = nullptr;

DeviceManager *DeviceManager::getInstance() {
    if(instance == nullptr){
        instance = new DeviceManager();
    }
    return instance;
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


//获取到设备列表则用最新的列表，获取不到设备列表则暂时使用上一次的列表直到该设备站点下线
void DeviceManager::updateDeviceList(){
    LOG_HLIGHT << "START TO updateDeviceList.....";
    qlibc::QData totalList;     //存储总列表
    std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
    smatch sm;
    for(auto& elem : siteNameSet){
        if(regex_match(elem, sm, regex("(.*):(.*)"))){
            string uuid = sm.str(1);
            string siteID = sm.str(2);
            if((siteID == BleSiteID || siteID == TvAdapterSiteID || siteID == ZigbeeSiteID || siteID == BtDeviceSiteID)){   //设备类站点
                qlibc::QData deviceRequest, deviceRes;
                deviceRequest.setString("service_id", "get_device_list");
                deviceRequest.setValue("request", Json::nullValue);
                SiteRecord::getInstance()->sendRequest2Site(sm.str(0), deviceRequest, deviceRes);       //获取设备列表
                if(deviceRes.getInt("code") != 0){
                    LOG_RED << "===>filed to get deviceList from <" << sm.str(0) << ">.......";
                    std::lock_guard<std::recursive_mutex> lg(Mutex);
                    auto pos = siteDeviceListMap.find(sm.str(0));
                    if(pos != siteDeviceListMap.end()){
                        qlibc::QData list = pos->second;
                        mergeList(list, totalList);
                    }
                }else{
                    LOG_HLIGHT << sm.str(0) << ": deviceListSize: " << deviceRes.getData("response").getData("device_list").size();
                    updateSiteDeviceListMap(sm.str(0), deviceRes.getData("response").getData("device_list"));
                    qlibc::QData list = addMacSource(deviceRes.getData("response").getData("device_list"), sm.str(0));  //给列表条目加入来源标签
                    mergeList(list, totalList);
                }
            }
        }
    }

    //存储新获取到的设备列表
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    deviceList_ = totalList;
    LOG_HLIGHT << "updateDeviceList END.....";
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

void DeviceManager::updateSiteDeviceListMap(string siteName, qlibc::QData deviceListData){
    std::lock_guard<recursive_mutex> lg(Mutex);
    auto pos = siteDeviceListMap.find(siteName);
    if(pos != siteDeviceListMap.end()){
        pos->second = deviceListData;
    }else{
        siteDeviceListMap.insert(std::make_pair(siteName, deviceListData));
    }
}
