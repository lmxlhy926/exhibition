//
// Created by 78472 on 2022/9/9.
//

#include "deviceTypeExtract.h"
#include "statusEvent.h"
#include "log/Logging.h"



string deviceTypeExtract::getDeviceType() {
   string productIndex = getProductIndex();
   ssize_t size = deviceTypeData.size();
   for(Json::ArrayIndex i = 0; i < size; ++i){
       qlibc::QData item = deviceTypeData.getArrayElement(i);
       Json::Value::Members members = item.getMemberNames();
       for(auto& key :members){
           if(key == productIndex){
               return item.getString("device_type");
           }
       }
   }
   return string("UNDEFINEDTYPE");
}

string deviceTypeExtract::getDeviceModel() {
    string productIndex = getProductIndex();
    ssize_t size = deviceTypeData.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceTypeData.getArrayElement(i);
        Json::Value::Members members = item.getMemberNames();
        for(auto& key :members){
            if(key == productIndex){
                return item.getString(key);
            }
        }
    }
    return string("UNDEFINEDTYPE");
}

string deviceTypeExtract::getProductIndex() {
    string productId;
    ReadBinaryString rs(deviceUUID);
    rs.readBytes(3).readBytes(productId, 2);
    return productId.substr(2, 2);
}