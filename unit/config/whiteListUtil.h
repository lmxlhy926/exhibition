
#include <map>
#include <string>
#include "qlibc/QData.h"
using namespace std;

using CategoryKeyMapType = std::map<string, std::map<string, Json::Value>>;
using PropertyMapType = std::map<string, Json::Value>;


//Json列表-->std::map<phone, std::map<deviceSn, Json::Value>>
CategoryKeyMapType JsonData2CategoryKeyMap(qlibc::QData& devices, string category, string uniqueKey);


//td::map<phone, std::map<deviceSn, Json::Value>>-->Json列表
qlibc::QData categoryKeyMap2JsonData(CategoryKeyMapType& phoneDeviceMap);


//清除特定类型的设备
void clearDevicesWithSpecificType(std::map<string, Json::Value>& devicesMap, string deviceType);


//清除相等的元素
void clearElementInReference(std::map<string, Json::Value>& referenceMap, std::map<string, Json::Value>& actualMap);


//复制设备到指定的设备map中
void copyDeviceElem(std::map<string, Json::Value>& source, std::map<string, Json::Value>& desination);


//获取经过处理的设备map
CategoryKeyMapType getHandledDeviceMap(CategoryKeyMapType& phoneDevicesMap, CategoryKeyMapType& phoneLocalDevicesMap);


//获取处理后的设备数据列表
qlibc::QData getHandledDeviceData(qlibc::QData& devices, qlibc::QData& localDevices);


//获取指定账号下的雷达列表
std::vector<string> getRadarVec(qlibc::QData& devices, string phone);


//属性列表转换为属性map
//std::map<keyID, Json::Value>
PropertyMapType properyData2PropertyMap(qlibc::QData& data, string key);


//属性map转换为属性列表
qlibc::QData propertyMap2PropertyData(PropertyMapType& propertyMap);


//获取roomMap
//有则替换，无则添加
PropertyMapType getSubstitudeRoomsMap(PropertyMapType& roomsMap, PropertyMapType& localRoomsMap);


qlibc::QData getSubstitudeRoomsData(qlibc::QData& rooms, qlibc::QData& localRooms);


//获取门信息
qlibc::QData getHandledAreasDoorsDataAfterRadarService(qlibc::QData& radarDevices, qlibc::QData& doors, qlibc::QData& localDoors,
                                                       const string& category, const string& uniqueKey);

//白名单合并
qlibc::QData getMergeWhiteList(std::map<unsigned long long, Json::Value> whiteListMap);
