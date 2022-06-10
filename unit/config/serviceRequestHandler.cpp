//
// Created by 78472 on 2022/5/11.
//

#include "serviceRequestHandler.h"
#include "paramconfig.h"
#include "common/httpUtil.h"
#include "siteService/service_site_manager.h"
using namespace servicesite;

int test_service_request_handler(const Request& request, Response& response) {
    // HTTP库已判断字符串能否转成 JSON

    // 请求的json字符串位于request.body
    auto request_json = json::parse(request.body);

    printf("request:\n%s\n", request_json.dump(4).c_str());

    // 服务反馈
    json response_json = {
            {"code", 0},
            {"error", "ok"},
            {"response", {
                             {"test_data", "from test_service_id_1"}
                     }}
    };
    response.set_content(response_json.dump(), "text/plain");


    return 0;
}


int sceneListRequest_service_request_handler(const Request& request, Response& response) {
    std::cout << "===>sceneListRequest_service_request_handler: " << request.body <<  std::endl;
    qlibc::QData sceneListRequest, sceneListResponse;
    qlibc::QData param;
    param.setString("familyCode", "XXXX");  //TODO 待定
    sceneListRequest.putData("param", param);
    sceneListRequest.setString("User-Agent", "curl");

    cloudUtil::getInstance()->ecb_httppost(SCENELIST_URL, sceneListRequest, sceneListResponse);

    qlibc::QData data;
    if(sceneListResponse.getInt("code") == 200){
        data.setInt("code", 0);
    }else
        data.setInt("code", 1);

    data.setString("error", sceneListResponse.getString("msg"));
    data.putData("response", sceneListResponse.getData("data"));

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

int subDeviceRegister_service_request_handler(const Request& request, Response& response) {
    std::cout << "===>subDeviceRegister_service_request_handler: " << request.body <<  std::endl;
    qlibc::QData requestData(request.body);

    qlibc::QData registerRequest, registerResponse;
    qlibc::QData param = requestData.getData("request");
    param.setString("domainID", configParamUtil::getInstance()->getBaseInfo().getString("domainID"));
    registerRequest.setString("User-Agent", "curl");
    registerRequest.putData("param", param);

    cloudUtil::getInstance()->ecb_httppost(SUBDEVICE_REGISTER_URL, registerRequest, registerResponse);

    qlibc::QData data;
    if(registerResponse.getInt("code") == 200){
        data.setInt("code", 0);
    }else{
        data.setInt("code", 1);
    }
    data.setString("error", registerResponse.getString("msg"));
    data.putData("response", qlibc::QData());
    response.set_content(data.toJsonString(), "text/json");

    return 0;
}

int domainIdRequest_service_request_handler(const Request& request, Response& response) {
    std::cout << "===>domainIdRequest_service_request_handler: " << request.body <<  std::endl;

    string domainId = configParamUtil::getInstance()->getBaseInfo().getString("domainID");
    qlibc::QData res;
    res.setString("domainId", domainId);

    qlibc::QData data;
    if(domainId.empty()){
        data.setInt("code", 1);
        data.setString("error", "domainID为空");
    }else{
        data.setInt("code", 0);
        data.setString("error", "ok");
    }
    data.putData("response", res);
    response.set_content(data.toJsonString(), "text/json");

    return 0;
}

int engineer_service_request_handler(mqttClient& mc, const Request& request, Response& response) {
    std::cout << "===>engineer_service_request_handler: " << request.body <<  std::endl;

    qlibc::QData requestData = qlibc::QData(request.body).getData("request");
    qlibc::QData registerRes;
    cloudUtil::getInstance()->tvRegister(mc, requestData, registerRes);

    qlibc::QData data;
    if(registerRes.getInt("code") == 200){
        data.setInt("code", 0);
    }else{
        data.setInt("code", 1);
    }
    data.setString("error", registerRes.getString("msg"));
    data.putData("response", qlibc::QData());

    response.set_content(data.toJsonString(), "text/json");

    return 0;
}


int whiteList_service_request_handler(const Request& request, Response& response){
    std::cout << "===>whiteList_service_request_handler: " << request.body <<  std::endl;

    qlibc::QData whiteListData = configParamUtil::getInstance()->getWhiteList();
    response.set_content(whiteListData.toJsonString(), "text/json");

    return 0;
}

int getAllDeviceList_service_request_handler(const Request& request, Response& response){
    //请求tvAdapter设备列表, 请求雷达、语音面板设备列表
    std::cout << "getAllDeviceList_service_request_handler" << request.body << std::endl;

    //像tvAdapter、zigbee、南向站点获取设备列表
    qlibc::QData deviceListRequest;
    qlibc::QData adapterResponse, zigBeeResponse, southResponse;
    deviceListRequest.setString("service_id", "get_device_list");
    deviceListRequest.putData("request", qlibc::QData());
    httpUtil::sitePostRequest(RequestIp, AdapterPort, deviceListRequest, adapterResponse);
    httpUtil::sitePostRequest(RequestIp, ZigBeeSitePort, deviceListRequest, zigBeeResponse);
    httpUtil::sitePostRequest(RequestIp, SouthPort, deviceListRequest, southResponse);

    //初始返回了列表为空
    qlibc::QData data, initRes;
    initRes.setValue("device_list", Json::Value(Json::arrayValue));
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", initRes);

    //组装设备列表
    qlibc::QData deviceListAdapter = adapterResponse.getData("response").getData("device_list");
    qlibc::QData deviceListZigbee = zigBeeResponse.getData("response").getData("device_list");
    qlibc::QData deviceListSouth = southResponse.getData("response").getData("device_list");

    qlibc::QData deviceList;
    for(int i = 0; i < deviceListAdapter.size(); i++){
        qlibc::QData deviceItem = deviceListAdapter.getArrayElement(i);
        deviceItem.setString("souceSite", "tv_adapter");
        deviceList.append(deviceItem);
    }
    for(int i = 0; i < deviceListZigbee.size(); i++){
        qlibc::QData deviceItem = deviceListZigbee.getArrayElement(i);
        deviceItem.setString("souceSite", "zigbee_light");
        deviceList.append(deviceItem);
    }
    for(int i = 0; i < deviceListSouth.size(); i++){
        qlibc::QData deviceItem = deviceListSouth.getArrayElement(i);
        deviceItem.setString("souceSite", "south");
        deviceList.append(deviceItem);
    }

    //写入获取到的设备列表
    qlibc::QData receiveRes;
    receiveRes.putData("device_list", deviceList);
    data.putData("response", receiveRes);


    response.set_content(data.toJsonString(), "text/json");

    return 0;
}

int tvSound_service_request_handler(const Request& request, Response& response){
    std::cout << "tvSound_service_request_handler" << request.body << std::endl;
    qlibc::QData requestData = qlibc::QData(request.body).getData("request");

    qlibc::QData data, content;
    data.setString("message_id", "getTvSound");
    content.setString("sn", requestData.getString("sn"));
    data.putData("content", content);

    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->publishMessage(TVSOUND_MESSAGE_ID, data.toJsonString());

    qlibc::QData retData;
    retData.setInt("code", 0);
    retData.setString("error", "ok");
    retData.putData("response", qlibc::QData());
    response.set_content(retData.toJsonString(), "text/json");
    return 0;
}
