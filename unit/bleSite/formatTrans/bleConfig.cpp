//
// Created by 78472 on 2022/6/7.
//

#include "bleConfig.h"
#include "qlibc/FileUtils.h"
#include <iostream>
#include "log/Logging.h"

bleConfig* bleConfig::instance = nullptr;

bleConfig* bleConfig::getInstance() {
    if(instance == nullptr)
        instance = new bleConfig(10);
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

QData bleConfig::getBleParamData() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    if(bleParamData.empty()){
        bleParamData.loadFromFile(FileUtils::contactFileName(dataDirPath, "data/bleCommand.json"));
    }
    return bleParamData;
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
void bleConfig::insertDeviceItem(string& deviceID){
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
    qlibc::QData newItem;
    newItem.setString("device_id", deviceID);
    deviceListArray.append(newItem);

    qlibc::QData saveData;
    saveData.putData("device_list", deviceListArray);
    saveDeviceListData(saveData);
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


void bleConfig::updateDeviceListProperty(){

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


void bleConfig::saveGroupListData(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    groupAddressData.setInitData(data);
    groupAddressData.saveToFile(FileUtils::contactFileName(dataDirPath, "data/groupAddress.json"), true);
}

bool bleConfig::serialInit(PackageMsgHandleFuncType func) {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    string serialPort = getSerialData().getString("serial");

    SerialParamStruct _sp;
    _sp.stopbits = 1;
    _sp.databits = 8;
    _sp.parity = 'N';
    _sp.baudrate = 115200;

    serial.reset(new TelinkDongle(serialPort, _sp));
    serial->registerPkgMsgFunc(std::move(func));

    if(!serial->startReadAndHandleSerial()){
        LOG_INFO << "===>startDongle failed, serialPort <" << serialPort << ">.......";
        serial.reset();
        return false;
    }else{
        LOG_INFO << "===>startDongle successfully, serialPort <" << serialPort << ">.......";
        return true;
    }
}

shared_ptr<TelinkDongle> bleConfig::getSerial() {
    std::lock_guard<std::recursive_mutex> lg(rMutex_);
    return serial;
}

void bleConfig::enqueue(std::function<void()> fn) {
    threadPool.enqueue(std::move(fn));
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
    itemColorTemperature.setInt("state_value", 6000);

    state_list.append(itemPower);
    state_list.append(itemLuminance);
    state_list.append(itemColorTemperature);

    return state_list;
}


