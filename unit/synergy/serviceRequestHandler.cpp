//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "deviceControl/contreteDeviceControl.h"
#include "paramConfig.h"

static const nlohmann::json okResponse = {
        {"code", 0},
        {"error", "ok"},
        {"response",{}}
};

static const nlohmann::json errResponse = {
        {"code", 1},
        {"error", "request format is not correct"},
        {"response",{}}
};


bool controlDeviceRightNow(qlibc::QData& message){
    std::cout << "received message: " << message.toJsonString() << std::endl;
    const DownCommandData downCommand(message);

    //构造请求体，获取设备列表
    qlibc::QData request, responseTvAdapter, responseZigbee;
    request.setString("service_id", "get_device_list");
    request.setValue("request", Json::nullValue);

    bool retTvAdapter = ControlBase::sitePostRequest(RequestIp, TvAdapterSitePort,request, responseTvAdapter);
    bool retZigbee = ControlBase::sitePostRequest(RequestIp, ZigbeeSitePort,request, responseZigbee);

    if(retTvAdapter || retZigbee){
        if(responseTvAdapter.getInt("code") == 0 || responseZigbee.getInt("code") == 0){
            std::cout << "===>get deviceList successfully" << std::endl;
            qlibc::QData deviceListTvAdapter = responseTvAdapter.getData("response").getData("device_list");
            qlibc::QData deviceListZigbee = responseZigbee.getData("response").getData("device_list");

            if(downCommand.deviceType == "light"){
                CommonControl comCtr;
                comCtr(downCommand, deviceListTvAdapter, TvAdapterSitePort);
                comCtr(downCommand, deviceListZigbee, ZigbeeSitePort);
            }
        }
    }
    return true;
}

int device_control_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        controlDeviceRightNow(requestBody);
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}






