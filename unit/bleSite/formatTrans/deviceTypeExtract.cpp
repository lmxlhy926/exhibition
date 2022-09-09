//
// Created by 78472 on 2022/9/9.
//

#include "deviceTypeExtract.h"
#include "statusEvent.h"
#include "bleConfig.h"
#include "log/Logging.h"

string deviceTypeExtract::getDeviceType() {
    string productId;
    ReadBinaryString rs(deviceUUID);
    rs.readBytes(3).readBytes(productId, 2);
    string productIndex = productId.substr(2, 2);
    return bleConfig::getInstance()->getDeviceTypeData().getString(productIndex);
}
