//
// Created by 78472 on 2022/5/16.
//

#include "configParamUtil.h"
#include "qlibc/FileUtils.h"

#include <utility>

configParamUtil* configParamUtil::instance = nullptr;

configParamUtil::configParamUtil(){}

configParamUtil *configParamUtil::getInstance() {
    if(instance == nullptr)
        instance = new configParamUtil;
    return instance;
}

void configParamUtil::setConfigPath(const string& configPath) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    dataDirPath = configPath;
}

string configParamUtil::getconfigPath() {
    return dataDirPath;
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
        whiteListData.loadFromFile(FileUtils::contactFileName(dataDirPath, "config/whiteList.json"));
    return whiteListData;
}

void configParamUtil::saveWhiteListData(QData &data) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    whiteListData.setInitData(data);
    whiteListData.saveToFile(FileUtils::contactFileName(dataDirPath, "config/whiteList.json"), true);
}




