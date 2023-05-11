//
// Created by 78472 on 2022/5/16.
//

#include "configParamUtil.h"
#include "qlibc/FileUtils.h"
#include <regex>

#include <utility>

configParamUtil* configParamUtil::instance = nullptr;

configParamUtil *configParamUtil::getInstance() {
    if(instance == nullptr)
        instance = new configParamUtil;
    return instance;
}

string configParamUtil::getconfigPath() {
    return dataDirPath;
}

void configParamUtil::setConfigPath(const string& configPath) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    dataDirPath = configPath;
}

QData configParamUtil::getBaseInfo() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(baseInfoData.empty()){
        baseInfoData.loadFromFile(FileUtils::contactFileName(dataDirPath, "config/baseInfo.json"));
    }
    return baseInfoData;
}

void configParamUtil::saveBaseInfo(QData& data) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    baseInfoData.setInitData(data);
    baseInfoData.saveToFile(FileUtils::contactFileName(dataDirPath, "config/baseInfo.json"), true);
}

QData configParamUtil::getRecordData() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(recordData.empty())
        recordData.loadFromFile(FileUtils::contactFileName(dataDirPath, "config/record.json"));
    return recordData;
}

void configParamUtil::saveRecordData(QData &data) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    recordData.setInitData(data);
    recordData.saveToFile(FileUtils::contactFileName(dataDirPath, "config/record.json"), true);
}

QData configParamUtil::getSecretFileNameData() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(secretFileNameData.empty())
        secretFileNameData.loadFromFile(FileUtils::contactFileName(dataDirPath, "config/secret/generateFile.json"));
    return secretFileNameData;
}

void configParamUtil::saveSecretFileNameData(QData& data) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    secretFileNameData.setInitData(data);
    secretFileNameData.saveToFile(FileUtils::contactFileName(dataDirPath, "config/secret/generateFile.json"), true);
}

QData configParamUtil::getMqttConfigData() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(mqttConfigData.empty())
        mqttConfigData.loadFromFile(FileUtils::contactFileName(dataDirPath, "config/mqttConfig.json"));
    return mqttConfigData;
}

QData configParamUtil::getCloudServerData(){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(cloudServerData.empty())
        cloudServerData.loadFromFile(FileUtils::contactFileName(dataDirPath, "config/httpConfig.json"));
    return cloudServerData;
}

QData configParamUtil::getWhiteList() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(whiteListData.empty())
        whiteListData.loadFromFile("/data/changhong/edge_midware/whitedevs-i.txt");
    return whiteListData;
}

void configParamUtil::saveWhiteListData(QData &data) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    whiteListData.setInitData(data);
    whiteListData.saveToFile("/data/changhong/edge_midware/whitedevs-i.txt", true);
}


void configParamUtil::saveOriginWhiteListData(QData &data){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    originWhiteListData.setInitData(data);
    originWhiteListData.saveToFile(FileUtils::contactFileName(dataDirPath, "config/originWhiteList.json"), true);
}

qlibc::QData configParamUtil::getSceneConfigFile(){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(sceneConfigFile.empty())
        sceneConfigFile.loadFromFile("/data/changhong/edge_midware/rules-config.txt");
    return sceneConfigFile;
}

void configParamUtil::saveSceneConfigFile(QData& data){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    sceneConfigFile.setInitData(data);
    sceneConfigFile.saveToFile("/data/changhong/edge_midware/rules-config.txt", true);
}

//获取面板信息
qlibc::QData configParamUtil::getPanelInfo(){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(panelInfoData.empty())
        panelInfoData.loadFromFile("/data/changhong/edge_midware/panelConfig.txt");
    return panelInfoData;
}

//设置面板信息
void configParamUtil::setPanelInfo(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    panelInfoData.setInitData(data);
    panelInfoData.saveToFile("/data/changhong/edge_midware/panelConfig.txt", true);
}

qlibc::QData configParamUtil::changePanelProperty(const qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    // string deviceMac;
    // string device_mac = panelInfoData.getString("device_mac");
    // regex sep("[:]+");
    // sregex_token_iterator end;
    // sregex_token_iterator p(device_mac.cbegin(), device_mac.cend(), sep, {-1});
    // for(; p != end; p++){
    //     deviceMac += p->str();
    // }

    if(panelInfoData.getString("device_mac") == data.getString("device_sn")){
        panelInfoData.setString("device_name", data.getString("device_name"));
        panelInfoData.setString("phone", data.getString("phone"));
        panelInfoData.asValue()["location"]["room_name"] = data.getString("room_name");
        panelInfoData.asValue()["location"]["room_no"] = data.getString("room_no");
        panelInfoData.saveToFile("/data/changhong/edge_midware/panelConfig.txt", true);
        return panelInfoData;
    }
    return qlibc::QData();
}


