
#include "whiteListUtil.h"

//Json列表-->std::map<phone, std::map<deviceSn, Json::Value>>
CategoryKeyMapType JsonData2CategoryKeyMap(qlibc::QData& devices, string category, string uniqueKey){
    CategoryKeyMapType devicesMap;
    for(Json::ArrayIndex i = 0; i < devices.size(); ++i){
        qlibc::QData item = devices.getArrayElement(i);
        string phone = item.getString(category);
        string deviceSn = item.getString(uniqueKey);
        if(!phone.empty() && !deviceSn.empty()){
            auto pos = devicesMap.find(phone);
            if(pos != devicesMap.end()){    //有则只添加数据
                pos->second.insert(std::make_pair(deviceSn, item.asValue()));
            }else{
                std::map<string, Json::Value> entry;
                entry.insert(std::make_pair(deviceSn, item.asValue()));
                devicesMap.insert(std::make_pair(phone, entry));    //无则创建条目
            }
        }
    }
    return devicesMap;
}

//td::map<phone, std::map<deviceSn, Json::Value>>-->Json列表
qlibc::QData categoryKeyMap2JsonData(CategoryKeyMapType& phoneDeviceMap){
    qlibc::QData deviceListData;
    for(auto pos = phoneDeviceMap.begin(); pos != phoneDeviceMap.end(); ++pos){
        std::map<string, Json::Value>& deviceMap = pos->second;
        for(auto position = deviceMap.begin(); position != deviceMap.end(); ++position){
            deviceListData.append(position->second);
        }
    }
    return deviceListData;
}

//清除特定类型的设备
void clearDevicesWithSpecificType(std::map<string, Json::Value>& devicesMap, string deviceType){
    for(auto position = devicesMap.begin(); position != devicesMap.end();){
            if(position->second["category_code"] == deviceType){
                position = devicesMap.erase(position);
            }else{
                ++position;
            }
        }
}

//清除相等的元素
void clearElementInReference(std::map<string, Json::Value>& referenceMap, std::map<string, Json::Value>& actualMap){
    for(auto pos = referenceMap.begin(); pos != referenceMap.end(); ++pos){
        if(actualMap.find(pos->first) != actualMap.end()){
            actualMap.erase(pos->first);
        }
    }
}

//复制设备到指定的设备map中
void copyDeviceElem(std::map<string, Json::Value>& source, std::map<string, Json::Value>& desination){
    for(auto pos = source.begin(); pos != source.end(); ++pos){
        desination.insert(*pos);
    }
}

//获取经过处理的设备map
CategoryKeyMapType getHandledDeviceMap(CategoryKeyMapType& phoneDevicesMap, CategoryKeyMapType& phoneLocalDevicesMap){
    //如果phoneDevicesMap为空
    if(phoneDevicesMap.empty()){  //不修改数据
         return phoneLocalDevicesMap;
    }else{
        for(auto phonePos = phoneDevicesMap.begin(); phonePos != phoneDevicesMap.end(); ++phonePos){
            string phone = phonePos->first;
            std::map<string, Json::Value>& devicesMap = phonePos->second;
            auto phoneLocalPos = phoneLocalDevicesMap.find(phone);
            if(phoneLocalPos != phoneLocalDevicesMap.end()){    //该账号存在
                std::map<string, Json::Value>& localDevicesMap = phoneLocalPos->second;
                for(auto& device : devicesMap){
                    auto findPosition = localDevicesMap.find(device.first);
                    if(findPosition != localDevicesMap.end()){  //有则替换
                        findPosition->second = device.second;
                    }else{  //无则插入
                        localDevicesMap.insert(device);
                    }
                }
            }else{  //本地没有该账号信息
                phoneLocalDevicesMap.insert(std::make_pair(phone, devicesMap));
            }
            
            //删除相同的元素
            for(auto position = phoneLocalDevicesMap.begin(); position != phoneLocalDevicesMap.end(); ++position){
                if(position->first != phone){
                    clearElementInReference(devicesMap, position->second);
                }
            }
        }
    }
    return phoneLocalDevicesMap;
}


//获取处理后的设备数据列表
qlibc::QData getHandledDeviceData(qlibc::QData& devices, qlibc::QData& localDevices){
    CategoryKeyMapType phoneDevicesMap = JsonData2CategoryKeyMap(devices, "phone", "device_sn");
    CategoryKeyMapType phoneLocalDevicesMap = JsonData2CategoryKeyMap(localDevices, "phone", "device_sn");
    CategoryKeyMapType handledDeviceMap = getHandledDeviceMap(phoneDevicesMap, phoneLocalDevicesMap);
    return categoryKeyMap2JsonData(handledDeviceMap);
}

//获取指定账号下的雷达列表
std::vector<string> getRadarVec(qlibc::QData& devices, string phone){
    std::vector<string> radarVec;
    Json::ArrayIndex size = devices.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = devices.getArrayElement(i);
        if(item.getString("phone") == phone && item.getString("category_code") == "radar" && !item.getString("device_sn").empty()){
            radarVec.push_back(item.getString("device_sn"));
        }
    }
    return radarVec;
}

#if 0
CategoryKeyMapType getHandledDoorsAreasMap(CategoryKeyMapType& radarSnKeyMap, CategoryKeyMapType& radarSnLocalKeyMap, std::vector<string>& radarSnVec){
    //如果phoneDevicesMap为空
    if(radarSnKeyMap.empty()){  
        //清除该账号下的雷达下的门和区域信息
        for(auto& radarSn : radarSnVec){
            if(radarSnLocalKeyMap.find(radarSn) != radarSnLocalKeyMap.end()){
                radarSnLocalKeyMap.erase(radarSn);
            }
        }
    }else{
        //对应替换
        for(auto pos = radarSnKeyMap.begin(); pos != radarSnKeyMap.end(); ++pos){
            string radarSn = pos->first;
            auto position = radarSnLocalKeyMap.find(radarSn);
            if(position != radarSnLocalKeyMap.end()){
                position->second = pos->second;
            }else{
                radarSnLocalKeyMap.insert(std::make_pair(radarSn, pos->second));
            }
        }
    }
    return radarSnLocalKeyMap;
}

//获取门数据
qlibc::QData getHandledDoorsData(qlibc::QData& doors, qlibc::QData& localDoors, std::vector<string>& radarSnVec){
    CategoryKeyMapType radarSnDoorsMap = JsonData2CategoryKeyMap(doors, "radarsn", "id");
    CategoryKeyMapType radarSnLocalDoorsMap = JsonData2CategoryKeyMap(localDoors, "radarsn", "id");
    CategoryKeyMapType handledDoorsMap = getHandledDoorsAreasMap(radarSnDoorsMap, radarSnLocalDoorsMap, radarSnVec);
    return categoryKeyMap2JsonData(handledDoorsMap);
}

//获取点位数据
qlibc::QData getHandledAreaData(qlibc::QData& doors, qlibc::QData& localDoors, std::vector<string>& radarSnVec){
    CategoryKeyMapType radarSnAreaMap = JsonData2CategoryKeyMap(doors, "radarsn", "area_id");
    CategoryKeyMapType radarSnLocalAreaMap = JsonData2CategoryKeyMap(localDoors, "radarsn", "area_id");
    CategoryKeyMapType handledAreaMap = getHandledDoorsAreasMap(radarSnAreaMap, radarSnLocalAreaMap, radarSnVec);
    return categoryKeyMap2JsonData(handledAreaMap);
}
#endif


//属性列表转换为属性map
//std::map<keyID, Json::Value>
PropertyMapType properyData2PropertyMap(qlibc::QData& data, string key){
    std::map<string, Json::Value> dataMap;
    for(Json::ArrayIndex i = 0; i < data.size(); ++i){
        qlibc::QData item = data.getArrayElement(i);
        string value = item.getString(key);
        if(!value.empty()){
            auto pos = dataMap.find(value);
            if(pos == dataMap.end()){   //沒有则添加，有则保持原数据
                dataMap.insert(std::make_pair(value, item.asValue()));
            }
        }
    }
    return dataMap;
}

//属性map转换为属性列表
qlibc::QData propertyMap2PropertyData(PropertyMapType& propertyMap){
    qlibc::QData propertyData;
    for(auto pos = propertyMap.begin(); pos != propertyMap.end(); ++pos){
        propertyData.append(pos->second);
    }
    return propertyData;
}


//获取roomMap
//有则替换，无则添加
PropertyMapType getSubstitudeRoomsMap(PropertyMapType& roomsMap, PropertyMapType& localRoomsMap){
    for(auto pos = roomsMap.begin(); pos != roomsMap.end(); ++pos){
        if(localRoomsMap.find(pos->first) != localRoomsMap.end()){  //有则替换
            localRoomsMap.erase(pos->first);
            localRoomsMap.insert(*pos);
        }else{
            localRoomsMap.insert(*pos); //无则添加
        }
    }
    return localRoomsMap;
}

qlibc::QData getSubstitudeRoomsData(qlibc::QData& rooms, qlibc::QData& localRooms){
    PropertyMapType roomsMap = properyData2PropertyMap(rooms, "roomNo");
    PropertyMapType localRoomsMap = properyData2PropertyMap(localRooms, "roomNo");
    PropertyMapType handledRoomsMap = getSubstitudeRoomsMap(roomsMap, localRoomsMap);
    return propertyMap2PropertyData(handledRoomsMap);
}


//获取门信息
qlibc::QData getHandledAreasDoorsDataAfterRadarService(qlibc::QData& devices, qlibc::QData& doors, qlibc::QData& localDoors,
                                                       const string& category, const string& uniqueKey){
    CategoryKeyMapType PhoneSnMap  = JsonData2CategoryKeyMap(devices, "phone", "device_sn");
    CategoryKeyMapType radarSnDoorsMap = JsonData2CategoryKeyMap(doors, category, uniqueKey);
    CategoryKeyMapType radarSnLocalDoorsMap = JsonData2CategoryKeyMap(localDoors, category, uniqueKey);
    
    for(auto pos = PhoneSnMap.begin(); pos != PhoneSnMap.end(); ++pos){
        string phone = pos->first;
        std::map<string, Json::Value>& devicesMap = pos->second;
        for(auto position = devicesMap.begin(); position != devicesMap.end(); ++position){
            string category_code = position->second["category_code"].asString();
            if(category_code == "radar"){
                string radarsn = position->first;
                string state = position->second["state"].asString();
                if(state == "ok"){
                    auto findPosition = radarSnDoorsMap.find(radarsn);
                    if(findPosition != radarSnDoorsMap.end()){  //推送下来的有数据
                        auto findLocalPosition = radarSnLocalDoorsMap.find(radarsn);
                        if(findLocalPosition != radarSnLocalDoorsMap.end()){   //本地存在则替换
                            findLocalPosition->second = findPosition->second;
                        }else{  //本地不存在则添加
                            radarSnLocalDoorsMap.insert(*findPosition);   
                        }
                    }
                }else if(state == "error"){
                    if(radarSnLocalDoorsMap.find(radarsn) != radarSnDoorsMap.end()){
                        radarSnLocalDoorsMap.erase(radarsn);
                    }
                }
            }
        }
    }
    return categoryKeyMap2JsonData(radarSnLocalDoorsMap);
}


void whiteListMerge(qlibc::QData& oldWhiteList, qlibc::QData& newWhiteList){
    qlibc::QData devices = newWhiteList.getData("info").getData("devices");
    qlibc::QData rooms = newWhiteList.getData("info").getData("rooms");
    qlibc::QData doors = newWhiteList.getData("info").getData("doors");
    qlibc::QData area_app = newWhiteList.getData("info").getData("area_app");

    qlibc::QData localDevices = oldWhiteList.getData("info").getData("devices");
    qlibc::QData localRooms = oldWhiteList.getData("info").getData("rooms");
    qlibc::QData localDoors = oldWhiteList.getData("info").getData("doors");
    qlibc::QData localAreas = oldWhiteList.getData("info").getData("area_app");

    qlibc::QData payload;
    payload.asValue()["info"]["devices"] = getHandledDeviceData(devices, localDevices).asValue();
    payload.asValue()["info"]["rooms"] = getSubstitudeRoomsData(rooms, localRooms).asValue();
    payload.asValue()["info"]["doors"] = getHandledAreasDoorsDataAfterRadarService(devices, doors, localDoors, "radarsn", "id").asValue();
    payload.asValue()["info"]["area_app"] = getHandledAreasDoorsDataAfterRadarService(devices, area_app, localAreas, "radarsn", "area_id").asValue();
    payload.setString("timeStamp", newWhiteList.getString("timeStamp"));

    oldWhiteList.setInitData(payload);
}

qlibc::QData getMergeWhiteList(std::map<unsigned long long, Json::Value> whiteListMap){
    qlibc::QData finalWhiteList;
    for(auto& elem : whiteListMap){
        qlibc::QData whiteListData(elem.second);
        whiteListMerge(finalWhiteList, whiteListData);
    }
    return finalWhiteList;
}


















