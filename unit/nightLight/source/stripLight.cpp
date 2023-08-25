
#include "stripLight.h"
#include "math.h"

void stripLight::addNewLogicStrip(Json::Value nightStripProperty){
    return;
}


void stripLight::handleRadarPoints(const RadarPointsType& allRoomAreaPoints){
    for(auto& roomAreaPoints : allRoomAreaPoints){
        if(logicalStripMap.find(roomAreaPoints.first) == logicalStripMap.end()){
            continue;
        }
        //此点所属的房间有逻辑灯带，找到逻辑灯带，进行处理
        auto start = logicalStripMap.lower_bound(roomAreaPoints.first);
        auto end = logicalStripMap.upper_bound(roomAreaPoints.first);
        for(auto pos = start; pos != end; ++pos){
            matchControl(roomAreaPoints.second, pos->second);
        }
    }
    return;
}


void stripLight::init(Json::Value nightStripProperty){
    return;
}

void stripLight::storeLogicStrip(){

}

CoordPointType stripLight::getCrossPoint(CoordPointType start, CoordPointType end, CoordPointType point){
    double k = (end.y - start.y) / (end.x - start.x);
    double b = start.y - k * start.x;
    double k0 = -1 / k;
    double b0 = point.y - k0 * point.x;
    CoordPointType crossPoint;
    crossPoint.x = (b0 - b) / (k - k0);
    crossPoint.y = k * crossPoint.x + b;
    return crossPoint;
}

bool stripLight::isInValidRange(CoordPointType point, CoordPointType crossPoint, double matchDistance){
    double realDistance = sqrt(pow((point.y - crossPoint.y), 2) + pow((point.x - crossPoint.x), 2));
    if(realDistance > matchDistance){
        return false;
    }
    return true;
}


uint stripLight::getCtrlChipIndex(CoordPointType start, CoordPointType crossPoint, double offsetDistance,
                          uint singleChipLength, uint startChipNum, uint endChipNum)
{
    double distance2Start = sqrt(pow((crossPoint.y - start.y), 2) + pow((crossPoint.x - start.x), 2));
    double distance2StartAfterAmend = distance2Start + offsetDistance;
    uint spacingChipNum = static_cast<uint>(distance2StartAfterAmend / singleChipLength);
    uint ctrolChipIndex = startChipNum + startChipNum;
    if(ctrolChipIndex > endChipNum){
        ctrolChipIndex = endChipNum;
    }
    return ctrolChipIndex;
}








