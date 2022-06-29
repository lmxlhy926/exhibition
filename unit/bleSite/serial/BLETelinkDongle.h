//
// Created by WJG on 2022-5-31.
//

#ifndef BLE_LIGHT_SITE_BLETELINKDONGLE_H
#define BLE_LIGHT_SITE_BLETELINKDONGLE_H

#include "CommonDongle.h"

class BLETelinkDongle : public CommonDongle{
public:
    explicit BLETelinkDongle(std::string _name);

    bool initDongle() override;
    bool setStartEndByte(unsigned char start, unsigned char end);
};

#endif //BLE_LIGHT_SITE_BLETELINKDONGLE_H
