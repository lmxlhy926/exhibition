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

bool GroupAddressMap::createGroup(string groupName) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    //组名必须唯一
    for(auto& elem : groupAddrMap){
        if(elem.second["group_name"].asString() == groupName){
            return false;
        }
    }

    //如果设备列表为空
    if(groupAddrMap.empty()){
        insert(groupName, 1);
        map2JsonDataAndSave2File();
        return true;
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
            insert(groupName, i + 1);
            map2JsonDataAndSave2File();
            return true;
        }
    }

    //没有空隙
    insert(groupName, addrVec.size() + 1);
    map2JsonDataAndSave2File();
    return true;
}

void GroupAddressMap::deleGroup(string &groupId) {
    std::lock_guard<recursive_mutex> lg(rMutex_);
    auto pos = groupAddrMap.find(groupId);
    if(pos != groupAddrMap.end()){
        groupAddrMap.erase(pos->first);
    }
    map2JsonDataAndSave2File();
}

bool GroupAddressMap::reNameGroup(string& groupId, string& groupName){
    std::lock_guard<recursive_mutex> lg(rMutex_);
    //如果组名已经存在则返回
    for(auto& elem : groupAddrMap){
        if(elem.second["group_name"].asString() == groupName){
            return false;
        }
    }

    auto pos = groupAddrMap.find(groupId);
    if(pos != groupAddrMap.end()){
        pos->second["group_name"] = groupName;
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
        data.setValue("group_name", elem.second["group_name"]);
        data.setValue("group_id", elem.first);
        data.setValue("device_list", elem.second["device_list"]);
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

void GroupAddressMap::insert(string &groupName, unsigned int intAddr) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData propertyItem;
    propertyItem.setString("group_name", groupName);

    groupAddrMap.insert(std::make_pair(intAddr2FullAddr(intAddr), propertyItem.asValue()));
}
