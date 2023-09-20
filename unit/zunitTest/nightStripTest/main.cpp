
#include <iostream>
#include <vector>
#include <thread>
#include <regex>
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "common/httpUtil.h"

using namespace std;

struct CoordPoint{
    double x;
    double y;
};

void loop(Json::Value targetList){
    Json::Value content, areaList, areaListItem;
    areaListItem["areaNo"] = "1";
    areaListItem["targetList"] = targetList;
    areaList.append(areaListItem);
    content["areaList"] = areaList;
    qlibc::QData request, response;
    request.setString("message_id", "reportTracingTargets");
    request.setValue("content", content);
    httpUtil::sitePostRequest("127.0.0.1", 9007, request, response);
    LOG_INFO << qlibc::QData(targetList).toJsonString();
}


void sendPoints(std::vector<CoordPoint> pointsVec){
    Json::Value targetList;
    for(auto& elem : pointsVec){
        Json::Value targetListItem;
        targetListItem["x"] = elem.x;
        targetListItem["y"] = elem.y;
        targetList.append(targetListItem);
    }
    loop(targetList);
}


int main(int argc, char* argv[]){
    while(true){
        for(double x = 6.35, y = 5.19; x > 4.12; x += -0.1){
            std::vector<CoordPoint> pointsVec;
            CoordPoint point;
            point.x = x;
            point.y = y;
            pointsVec.push_back(point);
            sendPoints(pointsVec);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        for(double x = 4.12, y = 5.19; x < 6.35; x += 0.1){
            std::vector<CoordPoint> pointsVec;
            CoordPoint point;
            point.x = x;
            point.y = y;
            pointsVec.push_back(point);
            sendPoints(pointsVec);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    }
   
    return 0;
}




























