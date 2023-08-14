#include "lightManage.h"


 void lightManage::flushLightsContaienr(Json::Value& deviceLists){
    std::lock_guard<std::recursive_mutex> lg(Mutex);

    return;
 }



void lightManage::calculateCoordPointAndControl(const string& pointsMessage){
    std::lock_guard<std::recursive_mutex> lg(Mutex);
    PointSequenceType pointSequence = trans2PointSequence(pointsMessage);
    for(auto& elem : lightsContainer){
        elem.calculateAndContol(pointSequence);
    }
    return;
}


PointSequenceType lightManage::trans2PointSequence(const string& pointMessage){
    PointSequenceType pointSequence;
    return pointSequence;
}























