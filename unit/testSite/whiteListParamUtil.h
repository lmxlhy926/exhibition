//
// Created by 78472 on 2022/5/16.
//

#ifndef EXHIBITION_WHITELISTPARAMUTIL_H
#define EXHIBITION_WHITELISTPARAMUTIL_H

#include <string>
#include "qlibc/QData.h"
#include <mutex>

using namespace std;
using namespace qlibc;

class whiteListParamUtil {
private:
    string dataDirPath;                 //文件配置路径
    QData whiteListData;                //云端下发的白名单信息
    static whiteListParamUtil *instance;
    std::recursive_mutex mutex_;

private:
    explicit whiteListParamUtil();

public:
    static whiteListParamUtil *getInstance();

    void setConfigPath(const string &configPath);

    string getconfigPath();

    QData getWhiteList();

    void saveWhiteListData(QData &data);
};

#endif //EXHIBITION_WHITELISTPARAMUTIL_H
