
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


void stripLight::updatePhysicalStrip(StripParamType strip){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    physicalStrip = strip;
}

void stripLight::handleRadarPoints(const RadarPointsType& allPoints){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    struct ControlPointType controlPoint{};  //将要控制的点位
    std::map<int, struct ControlPointType> chipIndex2Open;  //所有要控制的索引
    //获取全部要控制的索引
    for(auto elem : logicalStripMap){
        for(auto& logicalStrip : elem.second){
            auto pos = allPoints.find(logicalStrip.roomNo);
            if(pos != allPoints.end()){
                setChipIndexs2Open(logicalStrip, pos->second, chipIndex2Open);
            }
        }
    }
   
    if(chipIndex2Open.empty()){ //关灭灯带
        controlPoint.ctrlIndex = -1;
    }else{
        for(auto& elem : chipIndex2Open){
            if(elem.second.identity == latestControlPoint.identity){
                controlPoint = elem.second;
                break;
            }
        }
        if(controlPoint.ctrlIndex == -1){
            controlPoint = chipIndex2Open.begin()->second;
        }
    }

    //控制点相同，无需控制
    if(latestControlPoint.ctrlIndex == controlPoint.ctrlIndex){
        return;
    }
   
    Json::Value commandValue;
    commandValue["device_id"] = physicalStrip.device_id;
    commandValue["sourceSite"] = physicalStrip.sourceSite;

    if(controlPoint.ctrlIndex == -1){   //关灭灯带
        latestControlPoint = controlPoint;
        commandValue["controlPoints"] = {};
        sendBuffer::getInstance()->enque(commandValue);

    }else{
        //判断前进方向，增加或减少位移距离
        struct ControlPointType realControlPoint = getRealControlPoint(controlPoint);
        Json::Value indexValue(Json::arrayValue);
        indexValue.append(realControlPoint.ctrlIndex);
        commandValue["controlPoints"] = indexValue;
        sendBuffer::getInstance()->enque(commandValue);
    }
    LOG_PURPLE << "index2Control: " << qlibc::QData(commandValue).toJsonString(true);
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


struct ControlPointType stripLight::getCtrlChipIndex(LogicalStripType const& logicalStrip, CoordPointType const& point){
    printPoint("handlePoint", point);
    //得到交叉点
    CoordPointType crossPoint = getCrossPoint(logicalStrip, point);
    printPoint("crossPoint", crossPoint);
    //计算交叉点距离起始点的距离
    double crossPoint2Start = sqrt(pow((crossPoint.y - logicalStrip.start.y), 2) + pow((crossPoint.x - logicalStrip.start.x), 2));
    LOG_HLIGHT << "crossPoint2Start: " << crossPoint2Start;
    //计算交叉点距离点位的距离
    double crossPoint2Point = sqrt(pow((crossPoint.y - point.y), 2) + pow((crossPoint.x - point.x), 2));
    LOG_HLIGHT << "crossPoint2Point: " << crossPoint2Point;
    
    //判断落点和受控距离
    std::vector<double> absxVec{abs(logicalStrip.start.x), fabs(logicalStrip.end.x)};
    double min_x = *min_element(absxVec.begin(), absxVec.end());
    double max_x = *max_element(absxVec.begin(), absxVec.end());
    bool dropPointMatch = min_x <= fabs(crossPoint.x) && fabs(crossPoint.x) <= max_x;
    bool distanceMatch = crossPoint2Point <= physicalStrip.sensing_distance;

    //如果不是受控点，则控制编号返回-1
    if(!(dropPointMatch && distanceMatch)){
        LOG_INFO << "MATCH FAILED...";
        struct ControlPointType invalidCtrlPoint;
        invalidCtrlPoint.ctrlIndex = -1;
        return invalidCtrlPoint;
    }  
    
    //计算控制编号
    uint spacingChipNum = static_cast<uint>(crossPoint2Start / physicalStrip.led_spacing);
    uint ctrolChipIndex = logicalStrip.startChipNum + spacingChipNum;
    if(ctrolChipIndex > logicalStrip.endChipNum){
        ctrolChipIndex = logicalStrip.endChipNum;
    }

    LOG_PURPLE << "MATCH INDEX: " << ctrolChipIndex;
    LOG_INFO << "************************************************************";
    
    struct ControlPointType ctrlPoint{};
    ctrlPoint.device_id = point.device_id;
    ctrlPoint.roomNo = logicalStrip.roomNo;
    ctrlPoint.identity = point.identity;
    ctrlPoint.ctrlIndex = ctrolChipIndex;
    ctrlPoint.startChipNum = logicalStrip.startChipNum;
    ctrlPoint.endChipNum = logicalStrip.endChipNum;
    return ctrlPoint;
}


void stripLight::setChipIndexs2Open(LogicalStripType const& logicalStrip, std::vector<CoordPointType> const& points,
                                    std::map<int, ControlPointType>& chipIndex2Open){
    //找出有受控点的编号
    for(auto& point : points){
        struct ControlPointType controlPoint = getCtrlChipIndex(logicalStrip, point);
        if(controlPoint.ctrlIndex == -1) continue;
        auto pos = chipIndex2Open.find(controlPoint.ctrlIndex);
        if(pos == chipIndex2Open.end()){
            chipIndex2Open.insert(std::make_pair(controlPoint.ctrlIndex, controlPoint));
        }
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
    pointValue["identity"] = point.identity;
    pointValue["device_id"] = point.device_id;
    LOG_HLIGHT << msg << ": " << qlibc::QData(pointValue).toJsonString();
}


void stripLight::printIndex(const std::vector<int> index2Open){
    Json::Value printValue, openList, closeList;
    for(auto& elem : index2Open){   
        openList.append(elem);
    }
    printValue["openIndexList"] = openList;
    LOG_PURPLE << "index2Control: " << qlibc::QData(printValue).toJsonString(true);
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
    value["strip_length"] = physicalStrip.strip_length;
    value["lighting_range"] = physicalStrip.lighting_range;
    value["sensing_distance"] = physicalStrip.sensing_distance;
    value["led_spacing"] = physicalStrip.led_spacing;
    value["valid"] = physicalStrip.valid;
    value["device_id"] = physicalStrip.device_id;
    return value;
}


struct ControlPointType stripLight::getRealControlPoint(struct ControlPointType controlPoint){
    //判断前进方向，增加或减少位移距离
    struct ControlPointType backPoint = controlPoint;
    if(controlPoint.ctrlIndex > latestControlPoint.ctrlIndex){
        controlPoint.ctrlIndex += static_cast<uint>(physicalStrip.focus_offset / physicalStrip.led_spacing);
    }else{  
        controlPoint.ctrlIndex -= static_cast<uint>(physicalStrip.focus_offset / physicalStrip.led_spacing);
    }
    latestControlPoint = backPoint;

    int minIndex, maxIndex;
    if(controlPoint.startChipNum < controlPoint.endChipNum){
        minIndex = controlPoint.startChipNum;
        maxIndex = controlPoint.endChipNum;
    }else{
        minIndex = controlPoint.endChipNum;
        maxIndex = controlPoint.startChipNum;
    }
    if(controlPoint.ctrlIndex > maxIndex){
        controlPoint.ctrlIndex = maxIndex;
    }else if (controlPoint.ctrlIndex < minIndex){
        controlPoint.ctrlIndex = minIndex;
    }
    return controlPoint;
}



