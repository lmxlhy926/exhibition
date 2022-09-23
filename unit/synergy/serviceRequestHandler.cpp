//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "param.h"
#include "common/httpUtil.h"
#include "deviceControl/common.h"

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

qlibc::QData addSourceTag(qlibc::QData deviceList, string sourceSite){
    Json::ArrayIndex num = deviceList.size();
    qlibc::QData newDeviceList;
    for(Json::ArrayIndex i = 0; i < num; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        item.setString("sourceSite", sourceSite);
        newDeviceList.append(item);
    }
    return newDeviceList;
}

qlibc::QData getDeviceList(){
    qlibc::QData deviceRequest;
    deviceRequest.setString("service_id", "get_device_list");
    deviceRequest.setValue("request", Json::nullValue);

    qlibc::QData bleDeviceRes, zigbeeDeviceRes, tvDeviceRes;
    SiteRecord::getInstance()->sendRequest2Site(BleSiteID, deviceRequest, bleDeviceRes);
    SiteRecord::getInstance()->sendRequest2Site(ZigbeeSiteID, deviceRequest, zigbeeDeviceRes);
    SiteRecord::getInstance()->sendRequest2Site(TvAdapterSiteID, deviceRequest, tvDeviceRes);

    qlibc::QData deviceList;
    deviceList.append(addSourceTag(bleDeviceRes.getData("response").getData("device_list"), BleSiteID));
    deviceList.append(addSourceTag(zigbeeDeviceRes.getData("response").getData("device_list"), ZigbeeSiteID));
    deviceList.append(addSourceTag(tvDeviceRes.getData("response").getData("device_list"), TvAdapterSiteID));

   return deviceList;
}

void classify(qlibc::QData& controlData, qlibc::QData& bleDeviceList, qlibc::QData& zigbeeDeviceList, qlibc::QData& tvAdapterList){
    string sourceSite = controlData.getString("sourceSite");
    if(sourceSite == BleSiteID){
        bleDeviceList.append(controlData);
    }else if(sourceSite == ZigbeeSiteID){
        zigbeeDeviceList.append(controlData);
    }else if(sourceSite == TvAdapterSiteID){
        tvAdapterList.append(controlData);
    }
}

bool sendCmd(qlibc::QData& bleDeviceList, qlibc::QData& zigbeeDeviceList, qlibc::QData& tvAdapterList){
    qlibc::QData controlData, response;
    controlData.setString("service_id", "control_device");

    if(bleDeviceList.size() > 0){
        qlibc::QData list;
        list.putData("device_list", bleDeviceList);
        controlData.putData("request", list);
        SiteRecord::getInstance()->sendRequest2Site(BleSiteID, controlData, response);
    }
    if(zigbeeDeviceList.size() > 0){
        qlibc::QData list;
        list.putData("device_list", zigbeeDeviceList);

        controlData.putData("request", zigbeeDeviceList);
        SiteRecord::getInstance()->sendRequest2Site(BleSiteID, controlData, response);
    }
    if(tvAdapterList.size() > 0){
        controlData.putData("request", tvAdapterList);
        SiteRecord::getInstance()->sendRequest2Site(BleSiteID, controlData, response);
    }
    return true;
}


bool controlDeviceRightNow(qlibc::QData& message){
    std::cout << "received message: " << message.toJsonString() << std::endl;
    qlibc::QData deviceList = getDeviceList();
    DownCommandData downCommand(message);
    qlibc::QData controlData = downCommand.getContorlData(deviceList);

    qlibc::QData bleDeviceList, zigbeeDeviceList, tvAdapterList;
    classify(controlData, bleDeviceList, zigbeeDeviceList, tvAdapterList);
    sendCmd(bleDeviceList, zigbeeDeviceList, tvAdapterList);

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


int getDeviceList_service_handler(const Request& request, Response& response){
    qlibc::QData res;
    res.putData("device_list", getDeviceList());

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", res);

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

