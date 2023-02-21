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
    QData baseInfoData;                 //家庭基本参数信息
    QData recordData;                   //电视加入大白名单、电视注册结果
    QData secretFileNameData;           //获取文件名字
    QData mqttConfigData;               //获取mqtt配置信息
    QData cloudServerData;              //云端http服务器配置参数

    QData whiteListData;                //云端下发的转换后的白名单信息
    QData originWhiteListData;          //远端下发的原始白名单
    QData sceneConfigFile;              //场景配置文件
    static configParamUtil *instance;
    std::recursive_mutex mutex_;

private:
    explicit configParamUtil() = default;

public:
    static configParamUtil *getInstance();

    //设置文件配置路径
    void setConfigPath(const string &configPath);

    //获取文件配置路径
    string getconfigPath();

    //获取家庭基本参数信息
    QData getBaseInfo();

    //保存家庭基本参数信息
    void saveBaseInfo(QData &data);

    //获取注册结果
    QData getRecordData();

    //保存注册结果
    void saveRecordData(QData &data);

    //获取加密文件名
    QData getSecretFileNameData();

    //保存加密文件名
    void saveSecretFileNameData(QData &data);

    //获取mqtt服务器配置信息
    QData getMqttConfigData();

    //获取http服务器配置消息
    QData getCloudServerData();

    //获取白名单
    QData getWhiteList();

    //保存白名单
    void saveWhiteListData(QData &data);

    //保存未转换的白名单
    void saveOriginWhiteListData(QData &data);

    //获取场景配置文件
    qlibc::QData getSceneConfigFile();

    //保存场景配置文件
    void saveSceneConfigFile(QData& data);
};

#endif //EXHIBITION_CONFIGPARAMUTIL_H
