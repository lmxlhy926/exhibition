//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_BLECONFIG_H
#define EXHIBITION_BLECONFIG_H

#include <memory>
#include <functional>
#include "common/httplib.h"
#include "qlibc/QData.h"
#include "serial/telinkDongle.h"

using namespace qlibc;

class bleConfig {
private:
    string dataDirPath;                         //配置文件路径
    QData bleParamData;                         //蓝牙命令配置数据
    QData serialData;                           //串口配置数据
    QData deviceTypeData;                       //设备类型数据
    QData snAddressData;                        //蓝牙设备地址表
    QData groupAddressData;                     //组地址数据
    QData deviceListData;                       //蓝牙设备列表
    QData statusListData;                       //状态列表
    std::shared_ptr<TelinkDongle> serial;       //串口
    httplib::ThreadPool threadPool;             //线程池
    static bleConfig* instance;                 //静态对象
    std::recursive_mutex rMutex_;

    explicit bleConfig(size_t n) : threadPool(n){}
public:
    static bleConfig* getInstance();

    //设置配置路径
    void setConfigPath(const string &configPath);

    //获取配置路径
    string getconfigPath();

    //获取蓝牙命令配置数据
    QData getBleParamData();

    //获取串口配置数据
    QData getSerialData();

    //获取设备类型数据
    QData getDeviceTypeData();

    //获取device-mac记录表
    QData getSnAddrData();

    //存储device-mac记录表
    void saveSnAddrData(qlibc::QData& data);

    //获取设备列表
    QData getDeviceListData();

    //插入新绑定的设备条目
    void insertDeviceItem(string& deviceID, string& deviceType);

    //删除解绑的设备
    void deleteDeviceItem(string& deviceID);

    //获取config白名单信息，更新设备列表属性
    void updateDeviceListProperty();

    //存储设备列表
    void saveDeviceListData(qlibc::QData& data);

    //获取设备状态列表
    QData getStatusListData();

    //增加新绑定的设备的状态条目
    void insertDefaultStatus(string& deviceID);

    //删除解绑设备的状态条目
    void deleteStatusItem(string& deviceID);

    //收到设备状态上报后，更新设备状态列表
    void updateStatusListData(qlibc::QData& data);

    //存储设备状态列表
    void saveStatusListData(qlibc::QData& data);

    //获取组地址列表
    QData getGroupListData();

    //保存组地址列表
    void saveGroupListData(qlibc::QData& data);

    //初始化串口类，设置读取数据回调函数
    bool serialInit(PackageMsgHandleFuncType func);

    //获取串口操作对象
    shared_ptr<TelinkDongle> getSerial();

    //将函数加入线程池
    void enqueue(std::function<void()> fn);

private:
    //产生设备的默认状态
    qlibc::QData defaultStatus();
};


#endif //EXHIBITION_BLECONFIG_H
