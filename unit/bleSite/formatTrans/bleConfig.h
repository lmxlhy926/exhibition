//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_BLECONFIG_H
#define EXHIBITION_BLECONFIG_H

#include "qlibc/QData.h"
#include "serial/BLETelinkDongle.h"
#include <memory>
#include <functional>
#include "common/httplib.h"

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
    static bleConfig* instance;
    std::recursive_mutex mutex_;

private:
    explicit bleConfig(size_t n) : threadPool(n){}

public:
    static bleConfig* getInstance();

    void setConfigPath(const string &configPath);

    string getconfigPath();

    QData getBleParamData();

    QData getSerialData();

    QData getSnAddrData();

    void saveSnAddrData(qlibc::QData& data);

    bool serialInit(SerialReceiveFunc receiveFuc);

    shared_ptr<BLETelinkDongle> getSerial();

    void enqueue(std::function<void()> fn);
};


#endif //EXHIBITION_BLECONFIG_H
