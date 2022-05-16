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
    dataDirPath = configPath;
}

string configParamUtil::getconfigPath() {
    return dataDirPath;
}

QData configParamUtil::getBaseInfo() {
    if(baseInfoData.empty())
        baseInfoData.loadFromFile(FileUtils::contactFileName(dataDirPath, "baseInfo.json"));
    return baseInfoData;
}

void configParamUtil::saveBaseInfo(QData& data) {
    baseInfoData.setInitData(data);
    baseInfoData.saveToFile(FileUtils::contactFileName(dataDirPath, "baseInfo.json"));
}

QData configParamUtil::getRecordData() {
    if(recordData.empty())
        recordData.loadFromFile(FileUtils::contactFileName(dataDirPath, "record.json"));
    return recordData;
}

void configParamUtil::saveRecordData(QData &data) {
    recordData.setInitData(data);
    recordData.saveToFile(FileUtils::contactFileName(dataDirPath, "record.json"));
}

QData configParamUtil::getSecretFileNameData() {
    const string dir = FileUtils::contactFileName(dataDirPath, "secret");
    if(secretFileNameData.empty())
        secretFileNameData.loadFromFile(FileUtils::contactFileName(dir, "generateFile.json"));
    return secretFileNameData;
}

void configParamUtil::saveSecretFileNameData(QData& data) {
    const string dir = FileUtils::contactFileName(dataDirPath, "secret");
    secretFileNameData.setInitData(data);
    secretFileNameData.saveToFile(FileUtils::contactFileName(dir, "generateFile.json"));
}

QData configParamUtil::getMqttConfigData() {
    if(mqttConfigData.empty())
        mqttConfigData.loadFromFile(FileUtils::contactFileName(dataDirPath, "mqttConfig.json"));
    return mqttConfigData;
}

QData configParamUtil::getInterActiveAppData() {
    if(interactiveAppData.empty())
        interactiveAppData.loadFromFile(FileUtils::contactFileName(dataDirPath, "interactionApp.json"));
    return interactiveAppData;
}



