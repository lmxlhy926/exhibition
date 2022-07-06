//
// Created by 78472 on 2022/5/16.
//

#include "whiteListParamUtil.h"
#include "qlibc/FileUtils.h"

#include <utility>

whiteListParamUtil* whiteListParamUtil::instance = nullptr;

whiteListParamUtil::whiteListParamUtil(){}

whiteListParamUtil *whiteListParamUtil::getInstance() {
    if(instance == nullptr)
        instance = new whiteListParamUtil;
    return instance;
}

void whiteListParamUtil::setConfigPath(const string& configPath) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    dataDirPath = configPath;
}

string whiteListParamUtil::getconfigPath() {
    return dataDirPath;
}


QData whiteListParamUtil::getWhiteList() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(whiteListData.empty()){
        whiteListData.loadFromFile(FileUtils::contactFileName(dataDirPath, "config/whiteList.json"));
    }
    qlibc::QData response;
    response.setInt("code", 0);
    response.setString("error", "ok");
    response.putData("response", whiteListData);

    return response;
}

void whiteListParamUtil::saveWhiteListData(QData &data) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    whiteListData.setInitData(data);
    whiteListData.saveToFile(FileUtils::contactFileName(dataDirPath, "config/whiteList.json"), true);
}




