//
// Created by 78472 on 2022/5/11.
//

#include "serviceRequestHandler.h"
#include "param.h"
#include "common/httpUtil.h"
#include "siteService/service_site_manager.h"
#include "util/mqttPayloadHandle.h"
#include "log/Logging.h"
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

void extractFromWhiteList(qlibc::QData& deviceList){
    qlibc::QData whiteList = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData whiteListDevices = whiteList.getData("info").getData("devices");
    size_t size = whiteListDevices.size();
    for(size_t i = 0; i < size; ++i){
        qlibc::QData ithData;
        whiteListDevices.getArrayElement(i, ithData);
        qlibc::QData data;
        data.setString("device_id", ithData.getString("device_sn"));
        data.setString("online_state", "online");

        deviceList.append(data);
    }
}


int sceneListRequest_service_request_handler(const Request& request, Response& response) {
    LOG_INFO << "===>sceneListRequest_service_request_handler: " << request.body;
    qlibc::QData sceneListRequest, sceneListResponse;
    qlibc::QData param;
    param.setString("familyCode", "did:chisid:0x88508ea0601591e2afc95b662a9b279e75ef3f95");  //TODO 待定
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
    LOG_INFO << "===>subDeviceRegister_service_request_handler: " << request.body;
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
    LOG_INFO << "===>domainIdRequest_service_request_handler: " << request.body;

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
    LOG_INFO << "===>engineer_service_request_handler: " << request.body;

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
    LOG_INFO << "===>whiteListCloud_service_request_handler: " << request.body;

//    string domainID = configParamUtil::getInstance()->getBaseInfo().getString("domainID");
//
//    qlibc::QData whiteListRequest, whiteListResponse;
//    qlibc::QData param;
//    param.setString("familyCode", domainID);
//    whiteListRequest.putData("param", param);
//    whiteListRequest.setString("User-Agent", "curl");
//
//    cloudUtil::getInstance()->ecb_httppost(WHITELIST_REQUEST_URL, whiteListRequest, whiteListResponse);
//
//    qlibc::QData data;
//    if(whiteListResponse.getInt("code") == 200){
//        string payloadString = whiteListResponse.getData("data").toJsonString();
//        qlibc::QData payload = mqttPayloadHandle::transform(payloadString.c_str(), payloadString.size());
//        configParamUtil::getInstance()->saveWhiteListData(payload);
//
//        data.setInt("code", 0);
//        data.setString("error", whiteListResponse.getString("msg"));
//        data.putData("response", payload);
//
//    }else{
//        data.setInt("code", 1);
//        data.setString("error", whiteListResponse.getString("msg"));
//        data.putData("response", qlibc::QData());
//    }

    qlibc::QData data;
    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", payload);

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

int getAllDeviceList_service_request_handler(const Request& request, Response& response){
    //请求tvAdapter设备列表, 请求雷达、语音面板设备列表
    LOG_INFO << "getAllDeviceList_service_request_handler" << request.body;

    //像tvAdapter、zigbee、南向站点获取设备列表
    qlibc::QData deviceListRequest;
    qlibc::QData adapterResponse, zigBeeResponse;
    deviceListRequest.setString("service_id", "get_device_list");
    deviceListRequest.putData("request", qlibc::QData());

    httpUtil::sitePostRequest(RequestIp, AdapterPort, deviceListRequest, adapterResponse);
    httpUtil::sitePostRequest(RequestIp, ZigBeeSitePort, deviceListRequest, zigBeeResponse);

    //初始列表为空
    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");

    //组装设备列表
    qlibc::QData deviceListAdapter = adapterResponse.getData("response").getData("device_list");
    qlibc::QData deviceListZigbee = zigBeeResponse.getData("response").getData("device_list");

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

    //加入白名单设备
    extractFromWhiteList(deviceList);

    //写入获取到的设备列表
    qlibc::QData receiveRes;
    receiveRes.putData("device_list", deviceList);
    data.putData("response", receiveRes);

    response.set_content(data.toJsonString(), "text/json");

    return 0;
}

int tvSound_service_request_handler(const Request& request, Response& response){
    LOG_INFO << "tvSound_service_request_handler" << request.body;
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