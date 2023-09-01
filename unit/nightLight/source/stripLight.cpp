
#include "stripLight.h"
#include "math.h"
#include <algorithm>

bool stripLight::addLogiclStrip(LogicalStripType strip){
    string roomNo = strip.roomNo;
    auto pos = logicalStripMap.find(roomNo);
    if(pos != logicalStripMap.end()){
        logicalStripMap.insert(std::make_pair(roomNo, strip));
        return true;
    }else{
        auto start = logicalStripMap.lower_bound(roomNo);
        auto end = logicalStripMap.upper_bound(roomNo);
        bool noMatch{true};
        for(auto pos = start; pos != end; ++pos){
            if(pointsEqual(strip.start, pos->second.start) && pointsEqual(strip.end, pos->second.end)){
                noMatch = false;
                break;
            }
        }
        if(noMatch){
            logicalStripMap.insert(std::make_pair(roomNo, strip));
            return true;
        }
        return false;
    }
}


//删除逻辑段
bool stripLight::deleteLogiclStrip(LogicalStripType strip){
    for(auto pos = logicalStripMap.begin(); pos != logicalStripMap.end(); ){
        if(pointsEqual(pos->second.start, strip.start) && pointsEqual(pos->second.end, strip.end)){
            pos = logicalStripMap.erase(pos);
        }else{
            ++pos;
        }
    }
    return true;
}

//获取逻辑灯带列表
qlibc::QData stripLight::getLogiclStripList(){

    return {};
}

void stripLight::handleRadarPoints(const RadarPointsType& allRoomAreaPoints){
    std::vector<uint> chipIndex2Open;
    for(auto& roomAreaPoints : allRoomAreaPoints){
        if(logicalStripMap.find(roomAreaPoints.first) == logicalStripMap.end()){
            continue;
        }
        //此点所属的房间有逻辑灯带，找到逻辑灯带，进行处理
        auto start = logicalStripMap.lower_bound(roomAreaPoints.first);
        auto end = logicalStripMap.upper_bound(roomAreaPoints.first);
        for(auto pos = start; pos != end; ++pos){
            setChipIndexs2Open(pos->second, roomAreaPoints.second, chipIndex2Open);
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


void stripLight::init(StripParamType stripParam, LogicalStripType logicalStrip){
    physicalStrip = stripParam;
    logicalStripMap.insert(std::make_pair(logicalStrip.roomNo, logicalStrip));
    return;
}

bool stripLight::pointsEqual(CoordPointType first, CoordPointType second){
    return first.x == second.x && first.y == second.y;
}


CoordPointType stripLight::getCrossPoint(LogicalStripType& logicalStrip, CoordPointType point){
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

int stripLight::getCtrlChipIndex(LogicalStripType& logicalStrip, CoordPointType point){
    CoordPointType crossPoint = getCrossPoint(logicalStrip, point);
    //计算交叉点距离起始点的距离
    double crossPoint2Start = sqrt(pow((crossPoint.y - logicalStrip.start.y), 2) + pow((crossPoint.x - logicalStrip.start.x), 2));
    //计算交叉点距离点位的距离
    double crossPoint2Point = sqrt(pow((crossPoint.y - point.y), 2) + pow((crossPoint.x - point.x), 2));

    //判断落点和受控距离
    std::vector<double> absxVec{abs(logicalStrip.start.x), fabs(logicalStrip.end.x)};
    double min_x = *min_element(absxVec.begin(), absxVec.end());
    double max_x = *max_element(absxVec.begin(), absxVec.end());
    bool dropPointMatch = min_x < fabs(crossPoint.x) && fabs(crossPoint.x) < max_x;
    bool distanceMatch = crossPoint2Point < logicalStrip.matchDistance;

    //如果不是受控点，则控制编号返回-1
    if(!(dropPointMatch && distanceMatch))  return -1;

    //计算控制编号
    double crossPoint2StartAmend = crossPoint2Start + logicalStrip.offsetDistance;
    uint spacingChipNum = static_cast<uint>(crossPoint2StartAmend / physicalStrip.singleChipLength);
    uint ctrolChipIndex = logicalStrip.startChipNum + spacingChipNum;
    if(ctrolChipIndex > logicalStrip.endChipNum){
        ctrolChipIndex = logicalStrip.endChipNum;
    }
    return ctrolChipIndex;
}


 void stripLight::setChipIndexs2Open(LogicalStripType& logicalStrip, std::vector<CoordPointType> points, std::vector<uint>& chipIndex2Open){
    //找出有受控点的编号
    for(auto& point : points){
        if(chipIndex2Open.size() > 3) break;
        int chipIndex = getCtrlChipIndex(logicalStrip, point);
        if(chipIndex == -1) continue;
        chipIndex2Open.push_back(chipIndex);
    }
 }


void stripLight::controlStrip(std::vector<uint> index2Open, std::vector<uint> index2Close){

    return;
}




