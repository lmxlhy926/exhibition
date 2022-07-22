//
// Created by WJG on 2022-5-31.
//

#ifndef BLE_LIGHT_SITE_BLETELINKDONGLE_H
#define BLE_LIGHT_SITE_BLETELINKDONGLE_H

#include "CommonDongle.h"

class BLETelinkDongle : public CommonDongle{
public:
    explicit BLETelinkDongle(std::string _name);

    //设置通信参数，设置起始结束字符，初始化串口配置
    bool initDongle() override;
};

#endif //BLE_LIGHT_SITE_BLETELINKDONGLE_H
