#include "lightManage.h"

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

    start_time = request.getString("start_time");
    end_time = request.getString("end_time");
    string execuObjName = request.getString("execuObjName");
    string stripDeviceId = request.getString("stripDeviceId");
    
    //有相应物理灯带存在，则加入逻辑灯带。没有物理灯带存在，则先创建物理灯带，然后加入逻辑灯带
    auto pos = stripLightContainer.find(stripDeviceId);
    if(pos != stripLightContainer.end()){
        pos->second.addExecuteObj(execuObjName, logicalStripVec);
    }else{
        stripLight sl(getStripParam(stripDeviceId, deviceList));
        sl.addExecuteObj(execuObjName, logicalStripVec);
        stripLightContainer.insert(std::make_pair(stripDeviceId, sl));
    }

    //保存设备列表
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
    }
    //保存设备列表
    storeStripLightsContainer();
    return;
}


qlibc::QData lightManage::getLogiclStripList(){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    Json::Value list;
    for(auto& elem : stripLightContainer){
        list.append(elem.second.getLogiclStripList());
    }
    Json::Value data;
    data["start_time"]=start_time;
    data["end_time"]=end_time;
    data["list"]=list;
    return qlibc::QData(data);
}


void lightManage::handleRadarPoints(qlibc::QData&  pointData){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    RadarPointsType pointSequence = trans2PointSequence(pointData);
    for(auto& elem : stripLightContainer){
        elem.second.handleRadarPoints(pointSequence);
    }
    return;
}

qlibc::QData lightManage::getDeviceList(){

    return {};
}

StripParamType lightManage::getStripParam(const string& device_id, qlibc::QData& deviceList){

    StripParamType sl;
    return sl;
}


RadarPointsType lightManage::trans2PointSequence(qlibc::QData& pointData){
    RadarPointsType pointSequence;
    return pointSequence;
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
    qlibc::QData data = getLogiclStripList();
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


