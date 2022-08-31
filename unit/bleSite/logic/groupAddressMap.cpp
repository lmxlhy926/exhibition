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

string GroupAddressMap::getGroupAddr(string groupName) {
    return getAddress(groupName);
}

void GroupAddressMap::addDevice2Group(string& groupName, string& deviceSn){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupName);
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

void GroupAddressMap::deleteGroupItem(string &groupName) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupName);
    if(pos != groupAddrMap.end()){
        groupAddrMap.erase(pos->first);
    }
    map2JsonDataAndSave2File();
}

qlibc::QData GroupAddressMap::getGroupList() {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    qlibc::QData data;
    for(auto& elem : groupAddrMap){
        data.setValue(elem.first, elem.second);
    }
    return data;
}

string GroupAddressMap::groupName2Address(string groupName) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    for(auto& elem : groupAddrMap){
        if(elem.first == groupName){
            return elem.second["group_address"].asString();
        }
    }
    return "";
}

string GroupAddressMap::groupAddr2GroupName(string groupAddr) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    for(auto& elem : groupAddrMap){
        if(elem.second["group_address"] == groupAddr){
            return elem.first;
        }
    }
    return "";
}

void GroupAddressMap::loadCache2Map() {
    qlibc::QData groupAddressData = bleConfig::getInstance()->getGroupListData();
    Json::Value::Members groupNameVec = groupAddressData.getMemberNames();
    for(auto& groupName : groupNameVec){
        qlibc::QData propertyItem = groupAddressData.getData(groupName);
        groupAddrMap.insert(std::make_pair(groupName, propertyItem.asValue()));
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

void GroupAddressMap::insert(string &groupName, unsigned int intAddr) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupName);
    if(pos != groupAddrMap.end()){
        groupAddrMap.erase(pos);
    }
    qlibc::QData propertyItem;
    propertyItem.setString("group_address", intAddr2FullAddr(intAddr));

    groupAddrMap.insert(std::make_pair(groupName, propertyItem.asValue()));
}

string GroupAddressMap::getAddress(string &groupName) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);

    auto pos = groupAddrMap.find(groupName);
    if( pos != groupAddrMap.end()){
        return pos->second["group_address"].asString();
    }

    //如果设备列表为空
    if(groupAddrMap.empty()){
        insert(groupName, 1);
        map2JsonDataAndSave2File();
        return intAddr2FullAddr(1);
    }

    //将数字地址存入vector并排序
    std::vector<int> addrVec;
    for(auto& elem : groupAddrMap){
        int unicastInt = stoi(elem.second["group_address"].asString().substr(2, 2),
                              nullptr, 16);
        addrVec.push_back(unicastInt);
    }
    sort(addrVec.begin(), addrVec.end(), less<>());

    //如果有空隙，则返回最小空隙值
    for(unsigned int i = 0; i < addrVec.size(); ++i){
        if(addrVec[i] !=  i + 1){
            insert(groupName, i + 1);
            map2JsonDataAndSave2File();
            return intAddr2FullAddr(i + 1);
        }
    }

    //没有空隙
    insert(groupName, addrVec.size() + 1);
    map2JsonDataAndSave2File();
    return intAddr2FullAddr(addrVec.size() + 1);
}
