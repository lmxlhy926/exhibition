//
// Created by 78472 on 2022/9/9.
//

#include "deviceTypeExtract.h"
#include "statusEvent.h"
#include "log/Logging.h"

string deviceTypeExtract::getDeviceModel() {
    string deviceModelCode = getDeviceModelCode();
    ssize_t size = deviceTypeData.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceTypeData.getArrayElement(i);
        Json::Value::Members members = item.getMemberNames();
        for(auto& key :members){
            if(key == deviceModelCode){
                return item.getString(key);
            }
        }
    }
    return string("undefined_model");
}

string deviceTypeExtract::getDeviceModelCode(){
    string productId;
    ReadBinaryString rs(deviceUUID);
    rs.readBytes(3).readBytes(productId, 2);
    return productId.substr(2, 2);
}

string deviceTypeExtract::getDeviceType() {
    string deviceModelCode = getDeviceModelCode();
    ssize_t size = deviceTypeData.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
       qlibc::QData item = deviceTypeData.getArrayElement(i);
       Json::Value::Members members = item.getMemberNames();
       for(auto& key :members){
           if(key == deviceModelCode){
               return item.getString("device_type");
           }
       }
    }
    return string("undefined_type");
}

string deviceTypeExtract::getDeviceTypeCode(){
    string deviceModelCode = getDeviceModelCode();
    ssize_t size = deviceTypeData.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceTypeData.getArrayElement(i);
        Json::Value::Members members = item.getMemberNames();
        for(auto& key :members){
            if(key == deviceModelCode){
                return item.getString("device_typeCode");
            }
        }
    }
    return string("");
}

void deviceTypeExtract::initDeviceTypeData() {
    Json::Value lightData;
    lightData["device_type"] = "LIGHT";
    lightData["device_typeCode"] = "7";
    lightData["10"] = "方形吸顶灯";
    lightData["11"] = "全彩分子灯";
    lightData["12"] = "彩光灯带";
    lightData["13"] = "彩光球泡灯";
    lightData["14"] = "全彩灯带";

    lightData["20"] = "双色筒灯";
    lightData["21"] = "双色射灯";
    lightData["22"] = "双色球泡灯";
    lightData["23"] = "双色蜡烛灯";
    lightData["24"] = "双色分子灯";
    lightData["25"] = "双色面板灯";
    lightData["26"] = "圆形吸顶灯";
    lightData["27"] = "双色灯带";
    lightData["28"] = "磁吸轨道泛光灯";
    lightData["29"] = "磁吸轨道灯格栅灯";
    lightData["2A"] = "磁吸轨道灯导航灯";
    lightData["2B"] = "磁吸轨道转角灯";
    lightData["2C"] = "磁吸轨道折叠灯";
    lightData["2D"] = "磁吸轨道吊线灯";

    Json::Value socketData;
    socketData["device_type"] = "SOCKET";
    socketData["device_typeCode"] = "8";
    socketData["30"] = "墙壁插座";
    socketData["31"] = "移动插座";
    socketData["32"] = "通断器";

    Json::Value switchData;
    switchData["device_type"]  = "SWITCH";
    switchData["device_typeCode"] = "9";
    switchData["40"] = "移动型照明开关";
    switchData["41"] = "墙壁型照明开关";
    switchData["42"] = "移动型情景开关";
    switchData["43"] = "墙壁型情景开关";
    switchData["44"] = "墙壁型双键开关";
    switchData["45"] = "移动版双键开关";
    switchData["46"] = "移动型单键开关";
    switchData["47"] = "移动版四键开关";
    switchData["48"] = "墙壁版四键开关";


    Json::Value sensorData;
    sensorData["device_type"] = "SENSOR";
    sensorData["device_typeCode"] = "10";
    sensorData["50"] = "门窗传感器";
    sensorData["51"] = "人体传感器";
    sensorData["52"] = "温湿度传感器";
    sensorData["53"] = "PM2.5传感器";
    sensorData["54"] = "光照传感器";
    sensorData["55"] = "烟雾传感器";
    sensorData["56"] = "燃气传感器";
    sensorData["57"] = "水浸传感器";
    sensorData["58"] = "人在传感器";

    Json::Value curtainData;
    curtainData["device_type"] = "CURTAIN";
    curtainData["device_typeCode"] = "11";
    curtainData["60"] = "智能窗帘";

    deviceTypeData.append(lightData).append(socketData).append(switchData).append(sensorData).append(curtainData);
}
