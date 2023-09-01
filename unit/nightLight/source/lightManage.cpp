#include "lightManage.h"

lightManage* lightManage::Instance = nullptr;
 
//保存夜灯灯带
void lightManage::addLogicalStrip(qlibc::QData& data){
    qlibc::QData request = data.getData("request");

}

//删除夜灯灯带
void lightManage::delLogicalStrip(qlibc::QData& data){
    qlibc::QData request = data.getData("request");

}


//获取夜灯灯带列表
qlibc::QData lightManage::getLogicalStripList(){
    return {};
}

void lightManage::handleRadarPoints(qlibc::QData&  pointData){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    RadarPointsType pointSequence = trans2PointSequence(pointData);
    for(auto& stripLight : stripLightContainer){
        stripLight.handleRadarPoints(pointSequence);
    }
    return;
}


RadarPointsType lightManage::trans2PointSequence(qlibc::QData& pointData){
    RadarPointsType pointSequence;
    return pointSequence;
}

void lightManage::loadStripLightsContainer(){


}

void lightManage::storeStripLightsContainer(){


}




















