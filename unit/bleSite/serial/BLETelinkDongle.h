//
// Created by WJG on 2022-5-31.
//

#ifndef BLE_LIGHT_SITE_BLETELINKDONGLE_H
#define BLE_LIGHT_SITE_BLETELINKDONGLE_H

#include "CommonDongle.h"

class BLETelinkDongle : public CommonDongle{
public:
    //指定串口名：注意windows下和unix下串口名称的不同
    explicit BLETelinkDongle(std::string _name);

    /*
     * 设置串口通信参数
     * 设备读起始结束字符
     */
    bool initDongle() override;
};

#endif //BLE_LIGHT_SITE_BLETELINKDONGLE_H
