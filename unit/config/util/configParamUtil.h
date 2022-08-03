//
// Created by 78472 on 2022/5/16.
//

#ifndef EXHIBITION_CONFIGPARAMUTIL_H
#define EXHIBITION_CONFIGPARAMUTIL_H

#include <string>
#include "qlibc/QData.h"
#include <mutex>

using namespace std;
using namespace qlibc;

/*
 *  存取配置参数，临时数据的工具类
 *      1. 取：如果对象中没有数据，则将文件中的数据读取到对象中。如果对象中有数据，则直接返回对象。
 *      2. 存：覆盖存储对象中的数据，并将数据写入到文件中
 */
class configParamUtil {
private:
    string dataDirPath;                 //文件配置路径
    QData baseInfoData;                 //基本信息
    QData recordData;                   //电视加入大白名单、电视注册结果
    QData secretFileNameData;           //获取文件名字
    QData mqttConfigData;               //获取mqtt配置信息
    QData cloudServerData;              //云端http服务器配置参数
    QData whiteListData;                //云端下发的转换后的白名单信息
    QData originWhiteListData;          //远端下发的原始白名单
    static configParamUtil *instance;
    std::recursive_mutex mutex_;

private:
    explicit configParamUtil() = default;

public:
    static configParamUtil *getInstance();

    void setConfigPath(const string &configPath);

    string getconfigPath();

    QData getBaseInfo();

    void saveBaseInfo(QData &data);

    QData getRecordData();

    void saveRecordData(QData &data);

    QData getSecretFileNameData();

    void saveSecretFileNameData(QData &data);

    QData getMqttConfigData();

    QData getCloudServerData();

    QData getWhiteList();

    void saveWhiteListData(QData &data);

    void saveOriginWhiteListData(QData &data);

};

#endif //EXHIBITION_CONFIGPARAMUTIL_H
