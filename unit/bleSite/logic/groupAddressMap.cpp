//
// Created by 78472 on 2022/8/29.
//

#include "groupAddressMap.h"
#include "snAddressMap.h"
#include "formatTrans/bleConfig.h"
#include <algorithm>
#include <vector>
#include <sstream>
#include "log/Logging.h"

GroupAddressMap* GroupAddressMap::instance = nullptr;

string GroupAddressMap::createGroup(qlibc::QData& property) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    //如果设备列表为空
    if(groupAddrMap.empty()){
        insert(property, 1);
        map2JsonDataAndSave2File();
        return intAddr2FullAddr(1);
    }

    //将数字地址存入vector并排序
    std::vector<int> addrVec;
    for(auto& elem : groupAddrMap){
        int unicastInt = stoi(elem.first.substr(0, 2),
                              nullptr, 16);
        addrVec.push_back(unicastInt);
    }
    sort(addrVec.begin(), addrVec.end(), less<>());

    //如果有空隙，则返回最小空隙值
    for(unsigned int i = 0; i < addrVec.size(); ++i){
        if(addrVec[i] !=  i + 1){
            insert(property, i + 1);
            map2JsonDataAndSave2File();
            return intAddr2FullAddr(i + 1);
        }
    }

    //没有空隙
    insert(property, addrVec.size() + 1);
    map2JsonDataAndSave2File();
    return intAddr2FullAddr(addrVec.size() + 1);
}

void GroupAddressMap::deleGroup(string &groupId) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupId);
    if(pos != groupAddrMap.end()){
        groupAddrMap.erase(pos->first);
    }
    map2JsonDataAndSave2File();
}

bool GroupAddressMap::reNameGroup(string& groupId, qlibc::QData& property){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupId);
    if(pos != groupAddrMap.end()){
        Json::Value::Members keys = property.getMemberNames();
        for(auto& key : keys){
            if(key != "device_list" && key != "group_id"){
                pos->second[key] = property.getValue(key);
            }
        }
    }
    map2JsonDataAndSave2File();
    return true;
}

void GroupAddressMap::addDevice2Group(string& groupId, string& deviceSn){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupId);
    if(pos != groupAddrMap.end()){
        if(pos->second["device_list"].isNull()){
            pos->second["device_list"] = Json::Value(Json::arrayValue);
            pos->second["device_list"].append(deviceSn);

        }else{
            qlibc::QData deviceList(pos->second["device_list"]);
            size_t deviceListSize = deviceList.size();
            auto find = false;
            for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
                if(deviceList.getArrayElement(i).asValue().asString() == deviceSn){
                    find = true;
                    break;
                }
            }
            if(!find){
                pos->second["device_list"].append(deviceSn);
            }
        }
        map2JsonDataAndSave2File();
    }
}

void GroupAddressMap::removeDeviceFromGroup(string& groupId, string& deviceSn){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupId);
    if(pos != groupAddrMap.end()){
        if(pos->second["device_list"].isNull()){
            return;

        }else{
            qlibc::QData deviceList(pos->second["device_list"]);
            size_t deviceListSize = deviceList.size();

            for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
                if(deviceList.getArrayElement(i).asValue().asString() == deviceSn){
                    Json::Value value;
                    pos->second["device_list"].removeIndex(i, &value);
                }
            }

        }
        map2JsonDataAndSave2File();
    }
}

void GroupAddressMap::removeDeviceFromAnyGroup(string& deviceSn){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    for(auto& elem : groupAddrMap){
        if(elem.second["device_list"].isNull()){
            continue;
        }else{
            qlibc::QData deviceList(elem.second["device_list"]);
            size_t deviceListSize = deviceList.size();
            for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
                if(deviceList.getArrayElement(i).asValue().asString() == deviceSn){
                    Json::Value value;
                    elem.second["device_list"].removeIndex(i, &value);
                }
            }
        }
    }
    map2JsonDataAndSave2File();
}

qlibc::QData GroupAddressMap::getGroupList() {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    qlibc::QData groupList;
    for(auto& elem : groupAddrMap){
        qlibc::QData data;
        Json::Value::Members keys = elem.second.getMemberNames();
        for(auto& key : keys){
            data.setValue(key, elem.second[key]);
        }
        data.setValue("group_id", elem.first);
        groupList.append(data);
    }

    qlibc::QData retData;
    retData.putData("group_list", groupList);
    return retData;
}

bool GroupAddressMap::isGroupExist(string& groupId){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupId);
    if(pos != groupAddrMap.end())
        return true;
    else
        return false;
}

string GroupAddressMap::groupName2GroupAddressId(string groupName) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    for(auto& elem : groupAddrMap){
        if(elem.second["group_name"] == groupName){
            return elem.first;
        }
    }
    return "";
}

string GroupAddressMap::groupAddressId2GroupName(string groupAddrId) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    for(auto& elem : groupAddrMap){
        if(elem.first == groupAddrId){
            return elem.second["group_name"].asString();
        }
    }
    return "";
}

void GroupAddressMap::loadCache2Map() {
    qlibc::QData groupAddressData = bleConfig::getInstance()->getGroupListData();
    Json::Value::Members groupIdVec = groupAddressData.getMemberNames();
    for(auto& groupId : groupIdVec){
        qlibc::QData propertyItem = groupAddressData.getData(groupId);
        groupAddrMap.insert(std::make_pair(groupId, propertyItem.asValue()));
    }
}

void GroupAddressMap::map2JsonDataAndSave2File() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData data;
    for(auto& elem : groupAddrMap){
        data.setValue(elem.first, elem.second);
    }

    bleConfig::getInstance()->saveGroupListData(data);
}

string GroupAddressMap::intAddr2FullAddr(unsigned int i) {
    stringstream ss;
    ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << i << "C0";
    return ss.str();
}

void GroupAddressMap::insert(qlibc::QData& property, unsigned int intAddr) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    groupAddrMap.insert(std::make_pair(intAddr2FullAddr(intAddr), property.asValue()));
}
