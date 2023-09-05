#include "service.h"
#include "log/Logging.h"
#include "qlibc/QData.h"
#include "source/lightManage.h"
#include "siteService/nlohmann/json.hpp"



static const nlohmann::json okResponse = {
        {"code", 0},
        {"error", "ok"},
        {"response",{}}
};

static const nlohmann::json errResponse = {
        {"code", 1},
        {"error", "failed"},
        {"response",{}}
};

 void radarMessageHandle(const Request& request){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "radarMessageHandle: " << requestBody.toJsonString();
    lightManage::getInstance()->handleRadarPoints(requestBody);
 }


//保存夜灯灯带
int saveStrip_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "saveStrip_service_request_handler: " << requestBody.toJsonString();
    lightManage::getInstance()->addExecuteObj(requestBody);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//删除夜灯灯带
int delStrip_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "delStrip_service_request_handler: " << requestBody.toJsonString();
    lightManage::getInstance()->delExecuteObj(requestBody);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//获取灯带列表
int getStripList_service_request_handler(const Request& request, Response& response){
     qlibc::QData requestBody(request.body);
    LOG_INFO << "getStripList_service_request_handler: " << requestBody.toJsonString();
    qlibc::QData data = lightManage::getInstance()->getLogiclStripList();
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}