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
    qlibc::QData content = requestBody.getData("content");
    lightManage::getInstance()->handleRadarPoints(content);
}


int radarPoint_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "radarPoint_service_request_handler: " << requestBody.toJsonString();
    qlibc::QData requestData = requestBody.getData("request");
    lightManage::getInstance()->handleRadarPoints(requestData);
    return 0;
}


//保存夜灯灯带
int saveStrip_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "saveStrip_service_request_handler: " << requestBody.toJsonString();
    qlibc::QData requestData = requestBody.getData("request");
    bool ret = lightManage::getInstance()->addExecuteObj(requestData);
    if(ret){
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


//删除夜灯灯带
int delStrip_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "delStrip_service_request_handler: " << requestBody.toJsonString();
    qlibc::QData requestData = requestBody.getData("request");
    lightManage::getInstance()->delExecuteObj(requestData);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//获取灯带列表
int getStripList_service_request_handler(const Request& request, Response& response){
     qlibc::QData requestBody(request.body);
    LOG_INFO << "getStripList_service_request_handler: " << requestBody.toJsonString();
    qlibc::QData ret;
    ret.setInt("code", 0);
    ret.setString("error", "ok");
    ret.putData("response", lightManage::getInstance()->getLogicalStripList());
    response.set_content(ret.toJsonString(), "text/json");
    return 0;
}