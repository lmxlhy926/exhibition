//
// Created by 78472 on 2022/6/7.
//

#include "bleConfig.h"
#include "qlibc/FileUtils.h"
#include <iostream>
#include <utility>
#include "log/Logging.h"
#include "snAddressMap.h"

bleConfig* bleConfig::instance = nullptr;

bleConfig* bleConfig::getInstance() {
    if(instance == nullptr)
        instance = new bleConfig(30);
    return instance;
}

void bleConfig::setConfigPath(const string& configPath) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    dataDirPath = configPath;
}

string bleConfig::getconfigPath() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    return dataDirPath;
}


QData bleConfig::getSerialData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(serialData.empty()){
        serialData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/serialConfig.json"));
    }
    return serialData;
}


QData bleConfig::getSnAddrData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(snAddressData.empty()){
        snAddressData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/snAddress.json"));
    }
    return snAddressData;
}

void bleConfig::clearSnAddressData(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData emptyData;
    snAddressData.setInitData(emptyData);
    snAddressData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/snAddress.json"), true);
}

void bleConfig::saveSnAddrData(qlibc::QData& data) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    snAddressData.setInitData(data);
    snAddressData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/snAddress.json"), true);
}

QData bleConfig::getDeviceListData(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(deviceListData.empty()){
        deviceListData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/deviceList.json"));
    }
    return deviceListData;
}

/*
 * 如果设备列表有该条目，则先删除该条目
 * 将新设备添加入设备列表中
 */
void bleConfig::insertDeviceItem(string& deviceID, qlibc::QData& property){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData deviceListArray = getDeviceListData().getData("device_list");
    size_t deviceListArraySize = deviceListArray.size();
    //如果设备列表中有该条目，则先将该条目删除
    for(size_t i = 0; i < deviceListArraySize; ++i){
        qlibc::QData deviceItem = deviceListArray.getArrayElement(i);
        if(deviceItem.getString("device_id") == deviceID){
            deviceListArray.deleteArrayItem(i);
            break;
        }
    }
    //提取设备的属性信息
    qlibc::QData newItem;
    newItem.setString("device_id", deviceID);
    newItem.setString("device_type", property.getString("device_type"));
    newItem.setString("device_typeCode", property.getString("device_typeCode"));
    newItem.setString("device_model", property.getString("device_model"));
    newItem.setString("device_modelCode", property.getString("device_modelCode"));
    newItem.putData("location", property.getData("location"));

    deviceListArray.append(newItem);

    qlibc::QData saveData;
    saveData.putData("device_list", deviceListArray);
    saveDeviceListData(saveData);
}


void bleConfig::insertGroupInfo(string& deviceID, string& groupName, string& groupAddress){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData deviceListArray = getDeviceListData().getData("device_list");
    size_t deviceListArraySize = deviceListArray.size();
    //如果设备列表中有该条目，则先将该条目删除
    for(size_t i = 0; i < deviceListArraySize; ++i){
        qlibc::QData deviceItem = deviceListArray.getArrayElement(i);
        if(deviceItem.getString("device_id") == deviceID){
            deviceListArray.deleteArrayItem(i);

            //增加组信息
            deviceItem.setString("group_name", groupName);
            deviceItem.setString("group_address", groupAddress);
            deviceListArray.append(deviceItem);

            //存储
            qlibc::QData saveData;
            saveData.putData("device_list", deviceListArray);
            saveDeviceListData(saveData);
            break;
        }
    }
}

void bleConfig::deleteGroupInfo(string& deviceID){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData deviceListArray = getDeviceListData().getData("device_list");
    size_t deviceListArraySize = deviceListArray.size();
    //如果设备列表中有该条目，则先将该条目删除
    for(size_t i = 0; i < deviceListArraySize; ++i){
        qlibc::QData deviceItem = deviceListArray.getArrayElement(i);
        if(deviceItem.getString("device_id") == deviceID){
            deviceListArray.deleteArrayItem(i);

            //删除组信息
            deviceItem.removeMember("group_name");
            deviceItem.removeMember("group_address");
            deviceListArray.append(deviceItem);

            //存储
            qlibc::QData saveData;
            saveData.putData("device_list", deviceListArray);
            saveDeviceListData(saveData);
            break;
        }
    }
}

void bleConfig::deleteDeviceItem(string& deviceID){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData device_list = getDeviceListData().getData("device_list");
    size_t deviceListSize = device_list.size();
    for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
        if(device_list.getArrayElement(i).getString("device_id") == deviceID){
            device_list.deleteArrayItem(i);
            break;
        }
    }

    qlibc::QData saveData;
    saveData.putData("device_list", device_list);
    saveDeviceListData(saveData);
}

void bleConfig::clearDeviceList(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData emptyData;
    deviceListData.setInitData(emptyData);
    deviceListData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/deviceList.json"), true);
}

void bleConfig::saveDeviceListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    deviceListData.setInitData(data);
    deviceListData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/deviceList.json"), true);
}

QData bleConfig::getStatusListData(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(statusListData.empty()){
        statusListData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/statusList.json"));
    }
    return statusListData;
}

/*
 *  新绑定一个设备后，需要插入一个初始状态条目
 *  如果该设备对应的条目存在，则先删除
 *  添加一个初始状态的条目
 */
void bleConfig::insertDefaultStatus(string& deviceID){
    qlibc::QData item;
    item.setString("device_id", deviceID);
    item.setString("online_state", "online");
    item.putData("state_list", defaultStatus());

    qlibc::QData statusList = getStatusListData().getData("device_list");
    size_t statusListSize = statusList.size();
    for(Json::ArrayIndex i = 0; i < statusListSize; ++i){
        if(statusList.getArrayElement(i).getString("device_id") == deviceID){
            statusList.deleteArrayItem(i);
            break;
        }
    }
    statusList.append(item);

    qlibc::QData saveData;
    saveData.putData("device_list", statusList);
    saveStatusListData(saveData);
}

void bleConfig::deleteStatusItem(string& deviceID){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData device_list = getStatusListData().getData("device_list");
    size_t deviceListSize = device_list.size();
    for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
        if(device_list.getArrayElement(i).getString("device_id") == deviceID){
            device_list.deleteArrayItem(i);
            break;
        }
    }

    qlibc::QData saveData;
    saveData.putData("device_list", device_list);
    saveStatusListData(saveData);
}

void bleConfig::updateStatusListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    string device_id = data.getString("device_id");
    string state_id = data.getString("state_id");

    qlibc::QData device_list = getStatusListData().getData("device_list");
    size_t deviceListSize = device_list.size();
    for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
        qlibc::QData deviceItem = device_list.getArrayElement(i);
        if(deviceItem.getString("device_id") == device_id){
            qlibc::QData state_list = deviceItem.getData("state_list");
            size_t statusListSize = state_list.size();
            for(Json::ArrayIndex j = 0; j < statusListSize; ++j){
                if(state_list.getArrayElement(j).getString("state_id") == state_id){
                    device_list.asValue()[i]["state_list"][j]["state_value"] = data.asValue()["state_value"];
                    break;
                }
            }
            break;
        }
    }

    qlibc::QData saveData;
    saveData.putData("device_list", device_list);
    saveStatusListData(saveData);
}

void bleConfig::clearStatusList(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData emptyData;
    statusListData.setInitData(emptyData);
    statusListData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/statusList.json"), true);
}

void bleConfig::saveStatusListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    statusListData.setInitData(data);
    statusListData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/statusList.json"), true);
}

QData bleConfig::getGroupListData(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(groupAddressData.empty()){
        groupAddressData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/groupAddress.json"));
    }
    return groupAddressData;
}

 void bleConfig::clearGroupList(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData emptyData;
    groupAddressData.setInitData(emptyData);
    groupAddressData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/groupAddress.json"), true);
 }

void bleConfig::saveGroupListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    groupAddressData.setInitData(data);
    groupAddressData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/groupAddress.json"), true);
}

QData bleConfig::getScanListData(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(scanListData.empty()){
        scanListData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/scanList.json"));
    }
    return scanListData;
}

void bleConfig::saveScanListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    scanListData.setInitData(data);
    scanListData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/scanList.json"), true);
}

void bleConfig::enqueue(std::function<void()> fn) {
    threadPool.enqueue(std::move(fn));
}

void bleConfig::storeNetKey(string str){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    netKey = std::move(str);
}

string bleConfig::getNetKey(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    return netKey;
}

void bleConfig::loadStatusFromFile(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData status;
    status.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/status.json"));

    qlibc::QData deviceList = status.getData("device_list");
    Json::ArrayIndex deviceListSize = deviceList.size();
    for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string address = SnAddressMap::getInstance()->deviceSn2Address(item.getString("device_id"));
        if(address.empty()) break;
        Json::Value statusValue;
        qlibc::QData stateList = item.getData("state_list");
        for(Json::ArrayIndex j = 0; j < stateList.size(); ++j){
            qlibc::QData stateItem = stateList.getArrayElement(j);
            statusValue[stateItem.getString("state_id")] = stateItem.getData("state_value").asValue();
        }
        devGrpValueMap.insert(std::make_pair(address, statusValue));
    }

    qlibc::QData groupList = status.getData("group_list");
    Json::ArrayIndex groupListSize = groupList.size();
    for(Json::ArrayIndex i = 0; i < groupListSize; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string address = item.getString("group_id");
        if(address.empty()) break;
        Json::Value statusValue;
        qlibc::QData stateList = item.getData("state_list");
        for(Json::ArrayIndex j = 0; j < stateList.size(); ++j){
            qlibc::QData stateItem = stateList.getArrayElement(j);
            statusValue[stateItem.getString("state_id")] = stateItem.getData("state_value").asValue();
        }
        devGrpValueMap.insert(std::make_pair(address, statusValue));
    }
}

void bleConfig::storeDevGrpStatus2File(){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    qlibc::QData device_list, group_list;
    for(auto& elem: devGrpValueMap){
        if(isDevAddress(elem.first)){
            qlibc::QData deviceItem = createStatusItem(Type::device, elem.first, elem.second["power"].asString(), 
                                                      elem.second["luminance"].asInt(), elem.second["color_temperature"].asInt());
            device_list.append(deviceItem);
        }else if(isGrpAddress(elem.first)){
            qlibc::QData deviceItem = createStatusItem(Type::group, elem.first, elem.second["power"].asString(), 
                                                      elem.second["luminance"].asInt(), elem.second["color_temperature"].asInt());
            group_list.append(deviceItem);
        }
    }

    qlibc::QData statusList;
    statusList.putData("device_list", device_list);
    statusList.putData("group_list", group_list);
    statusList.saveToFile(FileUtils::contactFileName(dataDirPath, "data/status.json"), true);
}

void bleConfig::storeLuminance_powerOn(const string& groupId){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = devGrpValueMap.find(groupId);
    if(pos != devGrpValueMap.end()){
        pos->second["luminance"] = pos->second["tempLuminance"];
        pos->second["power"] = "on";
    }else{
        Json::Value value;
        value["power"] = "on";
        devGrpValueMap.insert(std::make_pair(groupId, value));
    }
    storeDevGrpStatus2File();
}

void bleConfig::storeLuminance_powerOff(const string& groupId){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = devGrpValueMap.find(groupId);
    if(pos != devGrpValueMap.end()){
        pos->second["luminance"] = 0;
        pos->second["power"] = "off";
    }else{
        Json::Value value;
        value["power"] = "off";
        devGrpValueMap.insert(std::make_pair(groupId, value));
    }
    storeDevGrpStatus2File();
}

void bleConfig::storeLuminance(const string& groupId, int luminance){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = devGrpValueMap.find(groupId);
    if(pos != devGrpValueMap.end()){
        pos->second["luminance"] = luminance;
        pos->second["tempLuminance"] = luminance;
        if(luminance == 0){
             pos->second["power"] = "off";
        }else{
            pos->second["power"] = "on";
        }
    }else{
        Json::Value value;
        value["luminance"] = luminance;
        value["tempLuminance"] = luminance;
        if(luminance > 0){
            value["power"] = "on";
        }else{
            value["power"] = "off";
        }
        devGrpValueMap.insert(std::make_pair(groupId, value));
    }
    storeDevGrpStatus2File();
}

void bleConfig::storeColorTemperature(const string& groupId, int temperature){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = devGrpValueMap.find(groupId);
    if(pos != devGrpValueMap.end()){
        pos->second["color_temperature"] = temperature;
    }else{
        Json::Value value;
        value["color_temperature"] = temperature;
        devGrpValueMap.insert(std::make_pair(groupId, value));
    }
    storeDevGrpStatus2File();
}

Json::Value bleConfig::getLuminanceColorTemperature(const string& groupId){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    auto pos = devGrpValueMap.find(groupId);
    if(pos != devGrpValueMap.end()){
        return pos->second;
    }
    return {};
}

qlibc::QData bleConfig::defaultStatus() {
    qlibc::QData state_list;

    qlibc::QData itemPower;
    itemPower.setString("state_id", "power");
    itemPower.setString("state_value", "on");

    qlibc::QData itemLuminance;
    itemLuminance.setString("state_id", "luminance");
    itemLuminance.setInt("state_value", 65535);

    qlibc::QData itemColorTemperature;
    itemColorTemperature.setString("state_id", "color_temperature");
    itemColorTemperature.setInt("state_value", 6700);

    state_list.append(itemPower);
    state_list.append(itemLuminance);
    state_list.append(itemColorTemperature);

    return state_list;
}

bool bleConfig::isDevAddress(const string& address){
    return address.substr(2, 2) == "02";
}

bool bleConfig::isGrpAddress(const string& address){
    return address.substr(2, 2) == "C0";
}

qlibc::QData bleConfig::createStatusItem(Type type, const string& address, const string& onOff, int luminance, int color_temperature){              
    qlibc::QData itemPower;
    itemPower.setString("state_id", "power");
    itemPower.setString("state_value", onOff);

    qlibc::QData itemLuminance;
    itemLuminance.setString("state_id", "luminance");
    itemLuminance.setInt("state_value", luminance);
    
    qlibc::QData itemColorTemperature;
    itemColorTemperature.setString("state_id", "color_temperature");
    itemColorTemperature.setInt("state_value", color_temperature);

    qlibc::QData state_list;
    state_list.append(itemPower);
    state_list.append(itemLuminance);
    state_list.append(itemColorTemperature);

    qlibc::QData item;
    if(type == Type::group){
        item.setString("group_id", address);
        
    }else if(type == Type::device){
        item.setString("device_id", SnAddressMap::getInstance()->address2DeviceSn(address));
    }
    item.setString("online_state", "online");
    item.putData("state_list", state_list);
    return item;
}





