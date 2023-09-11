
#include "stripLight.h"
#include <math.h>
#include "sendBuffer.h"
#include <algorithm>
#include <bitset>
#include <sstream>
#include <iomanip>
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

    std::vector<uint> chipIndex2Close;
    for(auto& elem : chipOpendIndex){
        auto pos = find(chipIndex2Open.begin(), chipIndex2Open.end(), elem);
        if(pos == chipIndex2Open.end()){
            chipIndex2Close.push_back(elem);    //要关闭的编号
        }else{
            chipIndex2Open.erase(pos);  //剔除不需要打开的编号
        }
    }

    controlStrip(chipIndex2Open, chipIndex2Close);
    chipOpendIndex = chipIndex2Open;
    return;
}


CoordPointType stripLight::getCrossPoint(LogicalStripType const& logicalStrip, CoordPointType const& point){
    CoordPointType start = logicalStrip.start;
    CoordPointType end = logicalStrip.end;
    double k = (end.y - start.y) / (end.x - start.x);
    double b = start.y - k * start.x;
    double k0 = -1 / k;
    double b0 = point.y - k0 * point.x;
    CoordPointType crossPoint;
    crossPoint.x = (b0 - b) / (k - k0);
    crossPoint.y = k * crossPoint.x + b;
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
    bool dropPointMatch = min_x < fabs(crossPoint.x) && fabs(crossPoint.x) < max_x;
    bool distanceMatch = crossPoint2Point < physicalStrip.sensing_distance;

    //如果不是受控点，则控制编号返回-1
    if(!(dropPointMatch && distanceMatch))  return -1;

    //计算控制编号
    double crossPoint2StartAmend = crossPoint2Start + physicalStrip.focus_offset;
    uint spacingChipNum = static_cast<uint>(crossPoint2StartAmend / physicalStrip.led_spacing);
    uint ctrolChipIndex = logicalStrip.startChipNum + spacingChipNum;
    if(ctrolChipIndex > logicalStrip.endChipNum){
        ctrolChipIndex = logicalStrip.endChipNum;
    }
    return ctrolChipIndex;
}


void stripLight::controlStrip(std::vector<uint> index2Open, std::vector<uint> index2Close){
    Json::Value printValue, openList, closeList;
    for(auto& elem : index2Open){   
        openList.append(elem);
    }
    for(auto& elem : index2Close){
        closeList.append(elem);
    }
    printValue["openIndexList"] = openList;
    printValue["closeIndexList"] = closeList;
    LOG_PURPLE << "index2Ctrl: " << qlibc::QData(printValue).toJsonString(true);
    

    //聚合所有控制索引
    std::vector<uint> index2Ctrl;
    copy(index2Open.begin(), index2Open.end(), back_inserter(index2Ctrl));
    copy(index2Close.begin(), index2Close.end(), back_inserter(index2Ctrl));

    //拆分控制索引的高2位和低8位
    std::vector<string> high2ByteVec;
    std::vector<uint> lightsCtrlVec;
    for(auto& elem : index2Ctrl){
        std::bitset<10> uintBitset(elem);
        high2ByteVec.push_back(uintBitset.to_string().substr(0, 2));
        lightsCtrlVec.push_back((uintBitset & std::bitset<10>("0011111111")).to_ulong());
    }

    //高2为的16进制字符串表示
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
    uint size2Zero = 6 - index2Open.size() - index2Close.size();
    for(int i = 0; i < size2Zero; ++i){
        light2CtrlStr.append("00");
    }

    //构造控制命令字符串
    string command;
    command.append("E2");
    command.append("00000211");
    command.append(high2ByteStr);
    command.append(light2CtrlStr);

    sendBuffer::getInstance()->enque(command);
    return;
}


void stripLight::setChipIndexs2Open(LogicalStripType const& logicalStrip, std::vector<CoordPointType> const& points, std::vector<uint>& chipIndex2Open){
    //找出有受控点的编号
    for(auto& point : points){
        if(chipIndex2Open.size() > 3) break;
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
    LOG_PURPLE << msg << ": " << qlibc::QData(pointValue).toJsonString(true);
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

