
#include "stripLight.h"
#include <math.h>
#include "sendBuffer.h"
#include <algorithm>
#include <bitset>
#include <sstream>
#include <iomanip>
#include <iterator>
#include <iostream>
#include "log/Logging.h"


bool stripLight::addExecuteObj(string const& objName, std::vector<LogicalStripType> const& logicalStripVec){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    auto pos = logicalStripMap.find(objName);
    if(pos != logicalStripMap.end()){
        logicalStripMap.erase(pos);
        logicalStripMap.insert(std::make_pair(objName, logicalStripVec));
    }else{
        logicalStripMap.insert(std::make_pair(objName, logicalStripVec));
    }
    return true;
}

//删除逻辑段
bool stripLight::delExecuteObj(string const& objName){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    auto pos = logicalStripMap.find(objName);
    if(pos != logicalStripMap.end()){
        logicalStripMap.erase(pos);
    }
    return true;
}

//获取逻辑灯带列表
Json::Value stripLight::getLogicalStripList(){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    Json::Value list;
    for(auto& elem : logicalStripMap){
        Json::Value obj, logicalStrips;
        for(auto& logicalStrip : elem.second){
            logicalStrips.append(LogicalStripType2Value(logicalStrip));
        }
        obj["execuObjName"] = elem.first;
        obj["stripDeviceId"] = physicalStrip.device_id;
        obj["logicalStrips"] = logicalStrips;
        list.append(obj);
    }
    return list;
}


void stripLight::handleRadarPoints(const RadarPointsType& allPoints){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    std::vector<uint> chipIndex2Open;
    for(auto elem : logicalStripMap){
        for(auto& logicalStrip : elem.second){
            auto pos = allPoints.find(logicalStrip.roomNo);
            if(pos != allPoints.end()){
                setChipIndexs2Open(logicalStrip, pos->second, chipIndex2Open);
            }
        }
    }
    //控制点相同，无需控制
    if(chipIndex2Open == chipOpendIndex)    return;

    chipOpendIndex = chipIndex2Open;
    //将要控制的点位
    printIndex(chipIndex2Open);

    Json::Value controlPointsValue;
    for(auto elem : chipIndex2Open){
        controlPointsValue.append(elem);
    }
    Json::Value commandValue;
    commandValue["device_id"] = physicalStrip.device_id;
    commandValue["sourceSite"] = physicalStrip.sourceSite;
    commandValue["controlPoints"] = controlPointsValue;
    sendBuffer::getInstance()->enque(commandValue);
    return;
}


CoordPointType stripLight::getCrossPoint(LogicalStripType const& logicalStrip, CoordPointType const& point){
    //计算2中特殊曲线
    CoordPointType crossPoint;
    if(logicalStrip.start.y == logicalStrip.end.y){ //横线
        crossPoint.x = point.x;
        crossPoint.y = logicalStrip.start.y;

    }else if(logicalStrip.start.x == logicalStrip.end.x){   //竖线
        crossPoint.x = logicalStrip.start.x;
        crossPoint.y = point.y;

    }else{
        CoordPointType start = logicalStrip.start;
        CoordPointType end = logicalStrip.end;
        double k = (end.y - start.y) / (end.x - start.x);
        double b = start.y - k * start.x;
        LOG_HLIGHT << "logicalStripLine: " << k << " * x + " << b;
        double k0 = -1 / k;
        double b0 = point.y - k0 * point.x;
        LOG_HLIGHT << "crossLine:        " << k0 << " * x + " << b0;
        crossPoint.x = (b0 - b) / (k - k0);
        crossPoint.y = k * crossPoint.x + b;
    }
    return crossPoint;
}


int stripLight::getCtrlChipIndex(LogicalStripType const& logicalStrip, CoordPointType const& point){
    printPoint("handlePoint", point);
    //得到交叉点
    CoordPointType crossPoint = getCrossPoint(logicalStrip, point);
    printPoint("crossPoint", crossPoint);
    //计算交叉点距离起始点的距离
    double crossPoint2Start = sqrt(pow((crossPoint.y - logicalStrip.start.y), 2) + pow((crossPoint.x - logicalStrip.start.x), 2));
    //计算交叉点距离点位的距离
    double crossPoint2Point = sqrt(pow((crossPoint.y - point.y), 2) + pow((crossPoint.x - point.x), 2));
    
    //判断落点和受控距离
    std::vector<double> absxVec{abs(logicalStrip.start.x), fabs(logicalStrip.end.x)};
    double min_x = *min_element(absxVec.begin(), absxVec.end());
    double max_x = *max_element(absxVec.begin(), absxVec.end());
    bool dropPointMatch = min_x <= fabs(crossPoint.x) && fabs(crossPoint.x) <= max_x;
    bool distanceMatch = crossPoint2Point <= physicalStrip.sensing_distance;

    //如果不是受控点，则控制编号返回-1
    if(!(dropPointMatch && distanceMatch)){
        LOG_INFO << "MATCH FAILED...";
        return -1;
    }  
    
    //计算控制编号
    double crossPoint2StartAmend = crossPoint2Start + physicalStrip.focus_offset;
    uint spacingChipNum = static_cast<uint>(crossPoint2StartAmend / physicalStrip.led_spacing);
    uint ctrolChipIndex = logicalStrip.startChipNum + spacingChipNum;
    if(ctrolChipIndex > logicalStrip.endChipNum){
        ctrolChipIndex = logicalStrip.endChipNum;
    }

    LOG_HLIGHT << "crossPoint2Start: " << crossPoint2Start;
    LOG_HLIGHT << "crossPoint2Point: " << crossPoint2Point;
    LOG_PURPLE << "MATCH INDEX: " << ctrolChipIndex;
    LOG_INFO << "************************************************************";
    return ctrolChipIndex;
}


void stripLight::setChipIndexs2Open(LogicalStripType const& logicalStrip, std::vector<CoordPointType> const& points, std::vector<uint>& chipIndex2Open){
    //找出有受控点的编号
    for(auto& point : points){
        if(chipIndex2Open.size() >= 6) break;
        int chipIndex = getCtrlChipIndex(logicalStrip, point);
        if(chipIndex == -1) continue;
        chipIndex2Open.push_back(chipIndex);
    }
    return;
}


bool stripLight::pointsEqual(CoordPointType first, CoordPointType second){
    return first.x == second.x && first.y == second.y;
}


void stripLight::printPoint(const string& msg, const CoordPointType& point){
    Json::Value pointValue;
    pointValue["x"] = point.x;
    pointValue["y"] = point.y;
    LOG_HLIGHT << msg << ": " << qlibc::QData(pointValue).toJsonString();
}


void stripLight::printIndex(std::vector<uint> index2Open){
    Json::Value printValue, openList, closeList;
    for(auto& elem : index2Open){   
        openList.append(elem);
    }
    printValue["openIndexList"] = openList;
    LOG_PURPLE << "index2Control: " << qlibc::QData(printValue).toJsonString(true);
}


void stripLight::printIndex(const string& msg, std::vector<uint> indexVec){
    Json::Value value;
    for(auto& elem : indexVec){
        value.append(elem);
    }
    LOG_PURPLE << msg << ": " << qlibc::QData(value).toJsonString(true);
}


Json::Value stripLight::LogicalStripType2Value(LogicalStripType const& logicalStrip){
    Json::Value value;
    value["logicalStripName"] = logicalStrip.logicalStripName;
    value["roomNo"] = logicalStrip.roomNo;
    value["start_x"] = logicalStrip.start.x;
    value["start_y"] = logicalStrip.start.y;
    value["end_x"] = logicalStrip.end.x;
    value["end_y"] = logicalStrip.end.y;
    value["start_chipNum"] = logicalStrip.startChipNum;
    value["end_chipNum"] = logicalStrip.endChipNum;
    return value;
}


Json::Value stripLight::physicalStripType2Value(StripParamType const& physicalStrip){
    Json::Value value;
    value["brightness"] = physicalStrip.lightParam.night2Light_brightness;
    value["color_temperature"] = physicalStrip.lightParam.night2Light_color_temperature;
    value["switch_time"] = physicalStrip.lightParam.night2Light_swithTime;
    value["strip_length"] = physicalStrip.strip_length;
    value["lighting_range"] = physicalStrip.lighting_range;
    value["sensing_distance"] = physicalStrip.sensing_distance;
    value["led_spacing"] = physicalStrip.led_spacing;
    value["valid"] = physicalStrip.valid;
    value["device_id"] = physicalStrip.device_id;
    return value;
}

