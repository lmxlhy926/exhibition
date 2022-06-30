//
// Created by 78472 on 2022/6/7.
//

#ifndef EXHIBITION_BLECONFIGPARAM_H
#define EXHIBITION_BLECONFIGPARAM_H

#include "qlibc/QData.h"
#include "serial/BLETelinkDongle.h"
#include <memory>
#include <functional>

using namespace qlibc;

class bleConfigParam {
public:
    using SerialReceiveFunc = bool(unsigned char*, int);
private:
    string dataDirPath;                         //配置文件路径
    QData bleParamData;                         //蓝牙命令配置数据
    QData serialData;                           //串口配置数据
    std::shared_ptr<BLETelinkDongle> serial;    //串口
    static bleConfigParam* instance;
    std::recursive_mutex mutex_;

private:
    bleConfigParam() = default;

public:
    static bleConfigParam* getInstance();

    void setConfigPath(const string &configPath);

    string getconfigPath();

    QData getBleParamData();

    QData getSerialData();

    bool serialInit(SerialReceiveFunc receiveFuc);

    shared_ptr<BLETelinkDongle> getSerial();
};


#endif //EXHIBITION_BLECONFIGPARAM_H
