#include "lightManage.h"
#include "common/httpUtil.h"
#include "log/Logging.h"
#include <algorithm>

static const string STRIPLIST_PATH = "/data/changhong/edge_midware/stripDeviceList.json";

lightManage* lightManage::Instance = nullptr;
 
bool lightManage::addExecuteObj(qlibc::QData& data, qlibc::QData deviceList){
    if(deviceList.empty()){
        if(!getDeviceList(deviceList))  return false;
    }

    std::lock_guard<std::recursive_mutex> lg(Mutex);
    start_time = data.getString("start_time");
    end_time = data.getString("end_time");
    qlibc::QData list = data.getData("list");
    Json::ArrayIndex listSize = list.size();

    for(Json::ArrayIndex listIndex = 0; listIndex < listSize; ++listIndex){
        qlibc::QData ithData = list.getArrayElement(listIndex);
        qlibc::QData logicalStrips = ithData.getData("logicalStrips");
        Json::ArrayIndex size = logicalStrips.size();
        if(size == 0)   continue;

        std::vector<LogicalStripType> logicalStripVec;
        for(Json::ArrayIndex logicalStripListIndex = 0; logicalStripListIndex < size; ++logicalStripListIndex){
            try{
                LogicalStripType logicalStrip = logicalStripValue2Struct(logicalStrips.getArrayElement(logicalStripListIndex).asValue());
                logicalStripVec.push_back(logicalStrip);
            }catch(const exception& e){
                LOG_RED << "logicalStripValue2Struct exception: " << e.what();
            }
        }
        string execuObjName = ithData.getString("execuObjName");
        string stripDeviceId = ithData.getString("stripDeviceId");

        //有相应物理灯带存在，则加入逻辑灯带。没有物理灯带存在，则先创建物理灯带，然后加入逻辑灯带
        auto pos = stripLightContainer.find(stripDeviceId);
        if(pos != stripLightContainer.end()){
            pos->second.addExecuteObj(execuObjName, logicalStripVec);
        }else{
            StripParamType sp = getStripParam(stripDeviceId, deviceList);
            if(!sp.valid)   continue;
            stripLight sl(sp);
            sl.addExecuteObj(execuObjName, logicalStripVec);
            stripLightContainer.insert(std::make_pair(stripDeviceId, sl));
        }
    }

    storeStripLightsContainer();
    return  true;
}


void lightManage::delExecuteObj(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    qlibc::QData list = data.getData("list");
    Json::ArrayIndex listSize = list.size();
    for(Json::ArrayIndex index = 0; index < listSize; ++index){
        qlibc::QData ithData = list.getArrayElement(index);
        string execuObjName = ithData.getString("execuObjName");
        string stripDeviceId = ithData.getString("stripDeviceId");
        auto pos = stripLightContainer.find(stripDeviceId);
        if(pos != stripLightContainer.end()){
            pos->second.delExecuteObj(execuObjName);
            storeStripLightsContainer();
        }
    }
    return;
}


qlibc::QData lightManage::getLogicalStripList(){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    Json::Value list;
    for(auto& elem : stripLightContainer){
        Json::Value subList = elem.second.getLogicalStripList();
        Json::ArrayIndex subListSize = subList.size();
        for(Json::ArrayIndex i = 0; i < subListSize; ++i){
            list.append(subList[i]);
        }
    }
    Json::Value data;
    data["start_time"]=start_time;
    data["end_time"]=end_time;
    data["list"]=list;
    return qlibc::QData(data);
}


 void lightManage::updatePhysicalStrip(qlibc::QData& deviceList){


 }


void lightManage::handleRadarPoints(qlibc::QData&  pointData){
    if(!isInValidTime()){
        LOG_RED << "NOT IN VALID TIME....";
        return;
    }
    RadarPointsType pointSequence = trans2PointSequence(pointData);
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    for(auto& elem : stripLightContainer){
        elem.second.handleRadarPoints(pointSequence);
    }
    return;
}


bool lightManage::getDeviceList(qlibc::QData& deviceList){
    qlibc::QData deviceRequest, deviceResponse;
    deviceRequest.setString("service_id", "get_device_list");
    deviceRequest.setValue("request", Json::nullValue);
    int count{};
    while(count < 3){
        if(httpUtil::sitePostRequest("127.0.0.1", 9007, deviceRequest, deviceResponse)){
            deviceList = deviceResponse.getData("response").getData("device_list");
            return true;
        }else{
            ++count;
        }
    }
    return false;
}


bool lightManage::getWhiteList(qlibc::QData& whiteList){
    qlibc::QData deviceRequest, deviceResponse;
    deviceRequest.setString("service_id", "whiteListRequest");
    deviceRequest.setValue("request", Json::nullValue);
    int count{};
    while(count < 3){
        if(httpUtil::sitePostRequest("127.0.0.1", 9006, deviceRequest, deviceResponse)){
            whiteList = deviceResponse.getData("response");
            return true;
        }else{
            ++count;
        }
    }
    return false;
}


StripParamType lightManage::stripData2Struct(const Json::Value& data){
    StripParamType sp{};
    try{
        struct NightLightCtlParamType param;
        memset(&param, 0, sizeof(param));
        param.night2Light_brightness =          data["stripProperty"]["brightness"].asInt();
        param.night2Light_color_temperature =   data["stripProperty"]["color_temperature"].asInt();
        param.night2Light_swithTime =           data["stripProperty"]["switch_time"].asInt();
        param.light2Night_swithTime =           data["stripProperty"]["switch_time"].asInt();
        
        sp.lightParam = param;
        sp.device_id =          data["device_id"].asString();
        sp.strip_length =       data["stripProperty"]["strip_length"].asDouble();
        sp.lighting_range =     data["stripProperty"]["lighting_range"].asDouble();
        sp.sensing_distance =   data["stripProperty"]["sensing_distance"].asDouble();
        sp.led_spacing =        data["stripProperty"]["led_spacing"].asDouble();
        sp.focus_offset =       data["stripProperty"]["focus_offset"].asDouble();
        sp.valid = true;

    }catch(const exception& e){
        LOG_RED << "stripData2Struct failed: " << e.what();
        sp.valid = false;
    }
    return sp;
}


StripParamType lightManage::getStripParam(const string& device_id, qlibc::QData& deviceList){
    StripParamType sp{};
    Json::ArrayIndex size = deviceList.size();
    for(int i = 0; i < size; ++i){
        qlibc::QData ithData = deviceList.getArrayElement(i);
        string deviceId = ithData.getString("device_id");
        if(deviceId == device_id){
            return stripData2Struct(ithData.asValue());
        }
    }
    return sp;
}


LogicalStripType lightManage::logicalStripValue2Struct(Json::Value const& value){
    LogicalStripType logicalStrip;
    logicalStrip.logicalStripName = value["logicalStripName"].asString();
    logicalStrip.roomNo = value["roomNo"].asString();
    logicalStrip.start.x = value["start_x"].asDouble();
    logicalStrip.start.y = value["start_y"].asDouble();
    logicalStrip.end.x = value["end_x"].asDouble();
    logicalStrip.end.y = value["end_y"].asDouble();
    logicalStrip.startChipNum = value["start_chipNum"].asUInt();
    logicalStrip.endChipNum = value["end_chipNum"].asUInt();
    return logicalStrip;
}


void lightManage::loadStripLightsContainer(){
    qlibc::QData data, deviceList;
    data.loadFromFile(STRIPLIST_PATH);
    while(!getDeviceList(deviceList)){
        std::this_thread::sleep_for(std::chrono::seconds(2));
        LOG_RED << "CAN NOT GET DEVICELIST, TRY TO GET AGAIN IN 2 SECONDS...";
    }
    LOG_INFO << "data: " << data.toJsonString(true);
    LOG_INFO << "deviceList: " << deviceList.toJsonString(true);
    addExecuteObj(data, deviceList);
    return;
}


void lightManage::storeStripLightsContainer(){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    qlibc::QData data = getLogicalStripList();
    data.saveToFile(STRIPLIST_PATH);
}


bool lightManage::isInValidTime(){
    return true;
}


std::map<string, string> lightManage::getAreaRoomMap(qlibc::QData& data){
    std::map<string, string> areaRoomMap;
    qlibc::QData area_app_list = data.getData("info").getData("area_app");
    Json::ArrayIndex size = area_app_list.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData ithData = area_app_list.getArrayElement(i);
        string area_id = ithData.getString("area_id");
        string roomId = ithData.getString("roomId");
        if(!area_id.empty() && !roomId.empty()){
            auto pos = areaRoomMap.find(area_id);
            if(pos == areaRoomMap.end()){
                areaRoomMap.insert(std::make_pair(area_id, roomId));
            }
        }
    }
    return areaRoomMap;
}


string lightManage::areaNum2RoomNo(const string& areaNo, std::map<string, string> const& areaRoomMap){
    auto pos = areaRoomMap.find(areaNo);
    if(pos != areaRoomMap.end()){
        return pos->second;
    }
    return "";
}


std::vector<CoordPointType> lightManage::getCoordPointVec(qlibc::QData& targetList){
    std::vector<CoordPointType> coordPointVec;
    Json::ArrayIndex targetListSize = targetList.size();
    for(Json::ArrayIndex i = 0; i < targetListSize; ++i){
        qlibc::QData ithData = targetList.getArrayElement(i);
        double x = ithData.asValue()["x"].asDouble();
        double y = ithData.asValue()["y"].asDouble();
        CoordPointType cp;
        cp.x = x;
        cp.y = y;
        coordPointVec.push_back(cp);
    }
    return coordPointVec;
}


RadarPointsType lightManage::trans2PointSequence(qlibc::QData& pointData){
    RadarPointsType rp;
    qlibc::QData whiteList;
    if(!getWhiteList(whiteList)){
        LOG_RED << "trans2PointSequence: CAN NOT GET WHITELIST...";
        return rp;
    }
    std::map<string, string> areaRoomMap = getAreaRoomMap(whiteList);
    qlibc::QData areaList = pointData.getData("areaList");
    Json::ArrayIndex areaListSize = areaList.size();
    for(Json::ArrayIndex i = 0; i < areaListSize; ++i){
        qlibc::QData ithData = areaList.getArrayElement(i);
        string roomNo = areaNum2RoomNo(ithData.getString("areaNo"), areaRoomMap);
        qlibc::QData targetList = ithData.getData("targetList");
        std::vector<CoordPointType> coordPointVec = getCoordPointVec(targetList);
        auto pos = rp.find("roomNo");
        if(pos == rp.end()){
            rp.insert(std::make_pair(roomNo, coordPointVec));
        }else{
            copy(coordPointVec.begin(), coordPointVec.end(), back_inserter(pos->second));
        }
    }
    printPointSequence(rp);
    return rp;
}


void lightManage::printPointSequence(const RadarPointsType& radarPoints){
    Json::Value value;
    for(auto& elem: radarPoints){
       std::vector<CoordPointType> const& coordPointVec = elem.second;
       Json::Value pointList;
       for(auto& coordPoint: coordPointVec){
            Json::Value point;
            point["x"] = coordPoint.x;
            point["y"] = coordPoint.y;
            pointList.append(point);
       }
       value[elem.first] = pointList;
    }
    LOG_YELLOW << "radarPoints: " << qlibc::QData(value).toJsonString();
}



