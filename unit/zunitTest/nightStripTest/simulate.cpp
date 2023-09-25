
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

struct stripInfo{
    double start_x;
    double start_y;
    double end_x;
    double end_y; 
    uint num; 
    uint interval;
    string areaNo;
};

struct stripInfo data2Struct(qlibc::QData data){
    struct stripInfo info;
    info.start_x = data.asValue()["start_x"].asDouble();
    info.start_y = data.asValue()["start_y"].asDouble();
    info.end_x = data.asValue()["end_x"].asDouble();
    info.end_y = data.asValue()["end_y"].asDouble();
    info.num = data.asValue()["num"].asInt();
    info.interval = data.asValue()["interval"].asInt();
    info.areaNo = data.asValue()["areaNo"].asString();
    return info;
}


void sendMessage2SynergySite(Json::Value targetList, string areaNo){
    Json::Value content, areaList, areaListItem;
    areaListItem["areaNo"] = areaNo;
    areaListItem["targetList"] = targetList;
    areaList.append(areaListItem);
    content["areaList"] = areaList;
    qlibc::QData request, response;
    request.setString("message_id", "reportTracingTargets");
    request.setValue("content", content);
    httpUtil::sitePostRequest("127.0.0.1", 9007, request, response);
    LOG_INFO << qlibc::QData(targetList).toJsonString();
}

void sendPoint(double x, double y, string areaNo){
    std::vector<CoordPoint> pointsVec;
    CoordPoint point;
    point.x = x;
    point.y = y;
    pointsVec.push_back(point);

    Json::Value targetList;
    for(auto& elem : pointsVec){
        Json::Value targetListItem;
        targetListItem["x"] = elem.x;
        targetListItem["y"] = elem.y;
        targetListItem["identity"] = 1;
        targetList.append(targetListItem);
    }
    sendMessage2SynergySite(targetList, areaNo);
}


void strip_x_axis(struct stripInfo info){
    double k = (info.end_y - info.start_y) /(info.end_x - info.start_x);
    double forwardDelta = (info.end_x - info.start_x) / info.num;
    double backwardDelta = -forwardDelta;
    while(true){
        double x = info.start_x;
        double y = info.start_y;
        for(int i = 0; i < info.num; ++i){
            x += forwardDelta;
            y += k * forwardDelta;
            sendPoint(x, y, info.areaNo);
            std::this_thread::sleep_for(std::chrono::milliseconds(info.interval));
        }

        x = info.end_x;
        y = info.end_y;
        for(int i = 0; i < info.num; ++i){
            x += backwardDelta;
            y += k * backwardDelta;
            sendPoint(x, y, info.areaNo);
            std::this_thread::sleep_for(std::chrono::milliseconds(info.interval));
        }
    }
}


//灯带为水平方向
void strip_horizontal(struct stripInfo info){
    double forwardDelta = (info.end_x - info.start_x) / info.num;
    double backwardDelta = -forwardDelta;
    while(true){
        double x = info.start_x;
        double y = info.start_y;
        for(int i = 0; i < info.num; ++i){
            x += forwardDelta;
            sendPoint(x, y, info.areaNo);
            std::this_thread::sleep_for(std::chrono::milliseconds(info.interval));
        }

        x = info.end_x;
        y = info.end_y;
        for(int i = 0; i < info.num; ++i){
            x += backwardDelta;
            sendPoint(x, y, info.areaNo);
            std::this_thread::sleep_for(std::chrono::milliseconds(info.interval));
        }
        
    }
}


//灯带为竖直方向
void strip_vertical(struct stripInfo info){
    double forwardDelta = (info.end_y - info.start_y) / info.num;
    double backwardDelta = -forwardDelta;
    while(true){
        double x = info.start_x;
        double y = info.start_y;
        for(int i = 0; i < info.num; ++i){
            y += forwardDelta;
            sendPoint(x, y, info.areaNo);
            std::this_thread::sleep_for(std::chrono::milliseconds(info.interval));
        }

        x = info.end_x;
        y = info.end_y;
        for(int i = 0; i < info.num; ++i){
            y += backwardDelta;
            sendPoint(x, y, info.areaNo);
            std::this_thread::sleep_for(std::chrono::milliseconds(info.interval));
        }
    }
}


void pointTest(string filePath){
    qlibc::QData data;
    data.loadFromFile(filePath);
    Json::Value::Members members = data.getMemberNames();
    for(auto& keyMember : members){
        qlibc::QData array = data.getData(keyMember);
        Json::ArrayIndex size = array.size();
        for(Json::ArrayIndex i = 0; i < size; ++i){
            qlibc::QData request = array.getArrayElement(i);
            qlibc::QData response;
            httpUtil::sitePostRequest("127.0.0.1", 9007, request, response);
            LOG_INFO << request.toJsonString();
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }
    }
}



int main(int argc, char* argv[]){
    if(argc < 2){
        LOG_RED << "usage: porc [dirPath]";
        return -1;
    }
    string dirPath = argv[1];
    qlibc::QData data;
    data.loadFromFile(string(dirPath).append("/config.json"));
    bool strip_simulate = data.getData("strip_simulate").getBool("toggle");
    bool point_simulate = data.getBool("point_simulate");
    if(strip_simulate){
        string option = data.getData("strip_simulate").getString("option");
        struct stripInfo info = data2Struct(data.getData("strip_simulate").getData("strip"));
        if(option == "horizontal"){
            strip_horizontal(info);
        }else if (option == "vertical"){
            strip_vertical(info);
        }else{
            strip_x_axis(info);
        }
    }else if(point_simulate){
        pointTest(string(dirPath).append("/simulate.json"));
    }

    return 0;
}



