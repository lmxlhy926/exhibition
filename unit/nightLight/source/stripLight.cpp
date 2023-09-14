
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

    //记录需要保持的索引
    std::vector<uint> chipIndex2Saved;
    for(auto& elem : chipOpendIndex){
        auto pos = find(chipIndex2Open.begin(), chipIndex2Open.end(), elem);
        if(pos != chipIndex2Open.end()){
            chipIndex2Saved.push_back(elem);
        }
    }

    for(auto& elem : chipIndex2Saved){
        chipOpendIndex.erase(remove(chipOpendIndex.begin(), chipOpendIndex.end(), elem), chipOpendIndex.end()); //剔除需要保持的，剩下的是需要关闭的
        chipIndex2Open.erase(remove(chipIndex2Open.begin(), chipIndex2Open.end(), elem), chipIndex2Open.end()); //剔除已经打开的，剩下的是真正需要打开的
    }

    std::vector<uint> chipIndex2Close = chipOpendIndex;
    //记录所有打开的的索引
    chipOpendIndex = chipIndex2Saved;
    copy(chipIndex2Open.begin(), chipIndex2Open.end(), back_inserter(chipOpendIndex));

   
    //控制需要操作的索引
    controlStrip(chipIndex2Open, chipIndex2Close);
    return;
}


CoordPointType stripLight::getCrossPoint(LogicalStripType const& logicalStrip, CoordPointType const& point){
    CoordPointType start = logicalStrip.start;
    CoordPointType end = logicalStrip.end;
    double k = (end.y - start.y) / (end.x - start.x);
    double b = start.y - k * start.x;
    LOG_HLIGHT << "logicalStripLine: " << k << " * x + " << b;
    double k0 = -1 / k;
    double b0 = point.y - k0 * point.x;
    LOG_HLIGHT << "crossLine:        " << k0 << " * x + " << b0;
    CoordPointType crossPoint;
    crossPoint.x = (b0 - b) / (k - k0);
    crossPoint.y = k * crossPoint.x + b;
    printPoint("crossPoint", crossPoint);
    return crossPoint;
}


int stripLight::getCtrlChipIndex(LogicalStripType const& logicalStrip, CoordPointType const& point){
    printPoint("handlePoint", point);
    //得到交叉点
    CoordPointType crossPoint = getCrossPoint(logicalStrip, point);
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
        LOG_RED << "MATCH FAILED ...";
        LOG_INFO << "************************************************************";
        return -1;
    }  
    
    //计算控制编号
    double crossPoint2StartAmend = crossPoint2Start + physicalStrip.focus_offset;
    uint spacingChipNum = static_cast<uint>(crossPoint2StartAmend / physicalStrip.led_spacing);
    uint ctrolChipIndex = logicalStrip.startChipNum + spacingChipNum;
    if(ctrolChipIndex > logicalStrip.endChipNum){
        ctrolChipIndex = logicalStrip.endChipNum;
    }
    LOG_PURPLE << "MATCH INDEX: " << ctrolChipIndex;
    LOG_INFO << "************************************************************";
    return ctrolChipIndex;
}


void stripLight::setChipIndexs2Open(LogicalStripType const& logicalStrip, std::vector<CoordPointType> const& points, std::vector<uint>& chipIndex2Open){
    //找出有受控点的编号
    for(auto& point : points){
        if(chipIndex2Open.size() >= 3) break;
        int chipIndex = getCtrlChipIndex(logicalStrip, point);
        if(chipIndex == -1) continue;
        chipIndex2Open.push_back(chipIndex);
    }
    return;
}


void stripLight::controlStrip(std::vector<uint> index2Open, std::vector<uint> index2Close){
    //打印将要控制的索引
    printIndex(index2Open, index2Close);

    //聚合所有控制索引
    std::vector<uint> index2Ctrl;
    copy(index2Open.begin(), index2Open.end(), back_inserter(index2Ctrl));
    copy(index2Close.begin(), index2Close.end(), back_inserter(index2Ctrl));
    if(index2Ctrl.empty())  return;

    //拆分控制索引的高2位和低8位
    std::vector<string> high2ByteVec;
    std::vector<uint> lightsCtrlVec;
    for(auto& elem : index2Ctrl){
        std::bitset<10> uintBitset(elem);
        high2ByteVec.push_back(uintBitset.to_string().substr(0, 2));
        lightsCtrlVec.push_back((uintBitset & std::bitset<10>("0011111111")).to_ulong());
    }

    //高2位的16进制字符串表示
    string high2ByteBinaryStr;
    for(auto pos = high2ByteVec.crbegin(); pos != high2ByteVec.crend(); ++pos){
        high2ByteBinaryStr.append(*pos);
    }
    stringstream ss;
    ss << std::uppercase << std::hex << std::setw(4) << std::setfill('0') << std::bitset<16>(high2ByteBinaryStr).to_ulong();
    string high2ByteStr = ss.str();

    //所有待控制的索引的低8位字符串表示
    string  light2CtrlStr;
    for(auto& elem : lightsCtrlVec){
        stringstream ss;
        ss << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << elem;
        light2CtrlStr.append(ss.str());
    }
    //不需要控制的点位填补0
    uint size2Zero = 6 - index2Ctrl.size();
    for(int i = 0; i < size2Zero; ++i){
        light2CtrlStr.append("00");
    }

    //构造控制命令字符串
    string command;
    command.append("E2");
    command.append("00000211");
    command.append(high2ByteStr);
    command.append(light2CtrlStr);

    Json::Value commandValue;
    commandValue["device_id"] = physicalStrip.device_id;
    commandValue["sourceSite"] = physicalStrip.sourceSite;
    commandValue["payload"] = command;
    sendBuffer::getInstance()->enque(commandValue);

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


void stripLight::printIndex(std::vector<uint> index2Open, std::vector<uint> index2Close){
    Json::Value printValue, openList, closeList;
    for(auto& elem : index2Open){   
        openList.append(elem);
    }
    for(auto& elem : index2Close){
        closeList.append(elem);
    }
    printValue["openIndexList"] = openList;
    printValue["closeIndexList"] = closeList;
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

