#include "lightManage.h"
#include "common/httpUtil.h"
#include "log/Logging.h"
#include <algorithm>

static const string STRIPLIST_PATH = "";

lightManage* lightManage::Instance = nullptr;
 
void lightManage::addExecuteObj(qlibc::QData& data){
    qlibc::QData deviceList = getDeviceList();
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    qlibc::QData request = data.getData("request");
    qlibc::QData logicalStrips = request.getData("logicalStrips");
    Json::ArrayIndex size = logicalStrips.size();
    if(size == 0)   return;

    //封装LogicalStrip
    std::vector<LogicalStripType> logicalStripVec;
    for(Json::ArrayIndex i = 0; i < size; ++i){
        LogicalStripType logicalStrip = logicalStripValue2Struct(logicalStrips.getArrayElement(i).asValue());
        logicalStripVec.push_back(logicalStrip);
    }

    string execuObjName = request.getString("execuObjName");
    string stripDeviceId = request.getString("stripDeviceId");
    //有相应物理灯带存在，则加入逻辑灯带。没有物理灯带存在，则先创建物理灯带，然后加入逻辑灯带
    auto pos = stripLightContainer.find(stripDeviceId);
    if(pos != stripLightContainer.end()){
        pos->second.addExecuteObj(execuObjName, logicalStripVec);
    }else{
        StripParamType sp = getStripParam(stripDeviceId, deviceList);
        if(sp.valid == false){
            return;
        }
        stripLight sl(sp);
        sl.addExecuteObj(execuObjName, logicalStripVec);
        stripLightContainer.insert(std::make_pair(stripDeviceId, sl));
    }

    //保存设备列表
    start_time = request.getString("start_time");
    end_time = request.getString("end_time");
    storeStripLightsContainer();
    return;
}


void lightManage::delExecuteObj(qlibc::QData& data){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    qlibc::QData request = data.getData("request");
    string execuObjName = request.getString("execuObjName");
    string stripDeviceId = request.getString("stripDeviceId");
    auto pos = stripLightContainer.find(stripDeviceId);
    if(pos != stripLightContainer.end()){
        pos->second.delExecuteObj(execuObjName);
        storeStripLightsContainer();
    }
    return;
}


qlibc::QData lightManage::getLogicalStripList(){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    Json::Value list;
    for(auto& elem : stripLightContainer){
        list.append(elem.second.getLogicalStripList());
    }
    Json::Value data;
    data["start_time"]=start_time;
    data["end_time"]=end_time;
    data["list"]=list;
    return qlibc::QData(data);
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


bool lightManage::isInValidTime(){
    return true;
}


qlibc::QData lightManage::getDeviceList(){
    qlibc::QData deviceRequest, deviceResponse;
    deviceRequest.setString("service_id", "get_device_list");
    deviceRequest.setValue("request", Json::nullValue);
    httpUtil::sitePostRequest("127.0.0.1", 9007, deviceRequest, deviceResponse);
    return deviceResponse.getData("response").getData("device_list");
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


qlibc::QData lightManage::getWhiteList(){
    qlibc::QData deviceRequest, deviceResponse;
    deviceRequest.setString("service_id", "whiteListRequest");
    deviceRequest.setValue("request", Json::nullValue);
    httpUtil::sitePostRequest("127.0.0.1", 9006, deviceRequest, deviceResponse);
    return deviceResponse.getData("response");
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
            if(pos != areaRoomMap.end()){
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
    qlibc::QData whiteList = getDeviceList();
    std::map<string, string> areaRoomMap = getAreaRoomMap(whiteList);
    qlibc::QData areaList = pointData.getData("content").getData("areaList");
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
    return rp;
}


void lightManage::loadStripLightsContainer(){
    qlibc::QData data;
    data.loadFromFile(STRIPLIST_PATH);
    qlibc::QData list = data.getData("list");
    Json::ArrayIndex size = list.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = list.getArrayElement(i);
        addExecuteObj(item);
    }
    start_time = data.getString("start_time");
    end_time = data.getString("end_time");  
    return;
}


void lightManage::storeStripLightsContainer(){
    qlibc::QData data = getLogicalStripList();
    data.saveToFile(STRIPLIST_PATH);
}


LogicalStripType lightManage::logicalStripValue2Struct(Json::Value const& value){
    LogicalStripType logicalStrip;
    logicalStrip.logicalName = value["logicalStripName"].asString();
    logicalStrip.roomNo = value["roomNo"].asString();
    logicalStrip.start.x = value["start_x"].asDouble();
    logicalStrip.start.y = value["start_y"].asDouble();
    logicalStrip.end.x = value["end_x"].asDouble();
    logicalStrip.end.y = value["end_y"].asDouble();
    logicalStrip.startChipNum = value["start_ChipNum"].asUInt();
    logicalStrip.endChipNum = value["end_chipNum"].asUInt();
    return logicalStrip;
}



