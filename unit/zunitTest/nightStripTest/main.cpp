
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

void sendPoint(double x, double y){
    std::vector<CoordPoint> pointsVec;
    CoordPoint point;
    point.x = x;
    point.y = y;
    pointsVec.push_back(point);
    sendPoints(pointsVec);
}


void stripTest(qlibc::QData& data){
    double start_x = data.asValue()["strip"]["start_x"].asDouble();
    double start_y = data.asValue()["strip"]["start_y"].asDouble();
    double end_x = data.asValue()["strip"]["end_x"].asDouble();
    double end_y = data.asValue()["strip"]["end_y"].asDouble();
    uint num = data.asValue()["strip"]["num"].asInt();
    uint interval = data.asValue()["strip"]["interval"].asInt();

    double k = (end_y - start_y) /(end_x - start_x);
    double forwardDelta = (end_x - start_x) / num;
    double backwardDelta = (start_x - end_x) / num;
    while(true){
        double x = start_x;
        double y = start_y;
        for(int i = 0; i < num; ++i){
            x += forwardDelta;
            y += k * forwardDelta;
            sendPoint(x, y);
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }

        x = end_x;
        y = end_y;
        for(int i = 0; i < num; ++i){
            x += backwardDelta;
            y += k * backwardDelta;
            sendPoint(x, y);
            std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        }
    }
}


void pointTest(qlibc::QData& data){
    qlibc::QData simulationPoint = data.getData("simulationPoint");
    Json::ArrayIndex size = simulationPoint.size();
    for(Json::ArrayIndex i = 0; i < size; ++size){
        qlibc::QData request= simulationPoint.getArrayElement(i);
        qlibc::QData response;
        httpUtil::sitePostRequest("127.0.0.1", 9007, request, response);
        LOG_INFO << request.toJsonString();
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
}



int main(int argc, char* argv[]){
    qlibc::QData data;
    data.loadFromFile("/home/lhy/smarthome/exhibition/unit/zunitTest/nightStripTest/config.json");
    bool simulate_strip = data.getBool("simulate_strip");
    bool simulate_point = data.getBool("simulate_point");
    if(simulate_strip){
        stripTest(data);
    }else{
        pointTest(data);
    }
    return 0;
}




























