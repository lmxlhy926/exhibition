//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_BLECONFIG_H
#define EXHIBITION_BLECONFIG_H

#include <memory>
#include <functional>
#include "common/httplib.h"
#include "qlibc/QData.h"
#include "serial/BLETelinkDongle.h"

using namespace qlibc;

class bleConfig {
public:
    using SerialReceiveFunc = bool(unsigned char*, int);
private:
    string dataDirPath;                         //配置文件路径
    QData bleParamData;                         //蓝牙命令配置数据
    QData serialData;                           //串口配置数据
    QData snAddressData;                        //蓝牙设备地址表
    std::shared_ptr<BLETelinkDongle> serial;    //串口
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

    //获取device-mac记录表
    QData getSnAddrData();

    //存储device-mac记录表
    void saveSnAddrData(qlibc::QData& data);

    //初始化串口类，设置读取数据回调函数
    bool serialInit(SerialReceiveFunc receiveFuc);

    //获取串口操作对象
    shared_ptr<BLETelinkDongle> getSerial();

    //将函数加入线程池
    void enqueue(std::function<void()> fn);
};


#endif //EXHIBITION_BLECONFIG_H
