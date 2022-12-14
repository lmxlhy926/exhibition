//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_BLECONFIG_H
#define EXHIBITION_BLECONFIG_H

#include <memory>
#include <functional>
#include "http/httplib.h"
#include "qlibc/QData.h"

using namespace qlibc;

class bleConfig {
private:
    string dataDirPath;                         //配置文件路径
    QData serialData;                           //串口配置数据
    QData snAddressData;                        //蓝牙设备地址表
    QData groupAddressData;                     //组地址数据
    QData deviceListData;                       //蓝牙设备列表
    QData statusListData;                       //状态列表
    QData scanListData;                         //扫描设备列表
    string netKey;                              //网络key
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

    //获取串口配置数据
    QData getSerialData();

    //获取device-mac记录表
    QData getSnAddrData();

    //存储device-mac记录表
    void saveSnAddrData(qlibc::QData& data);

    //获取设备列表
    QData getDeviceListData();

    //插入新绑定的设备条目
    void insertDeviceItem(string& deviceID, qlibc::QData& property);

    //插入分组信息
    void insertGroupInfo(string& deviceID, string& groupName, string& groupAddress);

    //删除分组信息
    void deleteGroupInfo(string& deviceID);

    //删除解绑的设备
    void deleteDeviceItem(string& deviceID);

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

    //获取设备扫描列表
    QData getScanListData();

    //存储设备扫描列表
    void saveScanListData(qlibc::QData& data);

    //将函数加入线程池
    void enqueue(std::function<void()> fn);

    //存储网络key
    void storeNetKey(string str);

    //获取网络key
    string getNetKey();

private:
    //产生设备的默认状态
    qlibc::QData defaultStatus();
};


#endif //EXHIBITION_BLECONFIG_H
