//
// Created by 78472 on 2022/10/22.
//

#ifndef EXHIBITION_MDMCONFIG_H
#define EXHIBITION_MDMCONFIG_H

#include <string>
#include <mutex>
#include "qlibc/QData.h"


using namespace std;
using namespace qlibc;

class mdmConfig {
private:
    string dataDirPath;                    //配置文件路径
    QData mqttConfigData;                  //mqtt配置参数
    QData baseInfoData;                    //基本参数信息
    QData authInfoData;                    //授权信息
    static mdmConfig* instance;            //静态对象
    std::recursive_mutex rMutex_;

    explicit mdmConfig() = default;
public:
    static mdmConfig* getInstance();

    //设置加载路径
    void setConfigPath(const string &configPath);

    //获取配置路径
    string getconfigPath();

    //获取mqtt配置参数
    QData getMqttConfigData();

    //获取基本配置参数
    QData getBaseInfoData();

    //获取授权参数信息
    QData getAuthInfoData();

    //保存授权参数信息
    void saveAuthInfoData(qlibc::QData& data);
};


#endif //EXHIBITION_MDMCONFIG_H
