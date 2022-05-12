//
// Created by 78472 on 2022/5/11.
//

#include "serviceRequestHandler.h"

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

    qlibc::QData sceneListRequest;
    qlibc::QData sceneListResponse;
    qlibc::QData param;
    //TODO 待定
    param.setString("familyCode", "XXXX");
    sceneListRequest.putData("param", param);
    sceneListRequest.setString("User-Agent", "curl");

    cloudUtil::getInstance()->ecb_httppost(SCENELIST_URL, sceneListRequest, sceneListResponse);

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", sceneListResponse);

    response.set_content(data.toJsonString(), "text/plain");
    return 0;
}

int subDeviceRegister_service_request_handler(const Request& request, Response& response) {

    qlibc::QData requestData(request.body);
    qlibc::QData param = requestData.getData("request");
    param.setString("domainID", configParamUtil::getInstance()->getBaseInfo().getString("domainID"));
    qlibc::QData registerRequest;
    qlibc::QData registerResponse;
    registerRequest.setString("User-Agent", "curl");
    registerRequest.putData("param", param);

    cloudUtil::getInstance()->ecb_httppost(SUBDEVICE_REGISTER_URL, registerRequest, registerResponse);

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    response.set_content(data.toJsonString(), "text/plain");

    return 0;
}

int domainIdRequest_service_request_handler(const Request& request, Response& response) {

    string domainId = configParamUtil::getInstance()->getBaseInfo().getString("domainID");
    qlibc::QData res;
    res.setString("domainId", domainId);

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", res);
    response.set_content(data.toJsonString(), "text/plain");

    return 0;
}

int engineer_service_request_handler(const Request& request, Response& response) {
    qlibc::QData requestData = qlibc::QData(request.body).getData("request");
    qlibc::QData registerRes;
    cloudUtil::getInstance()->tvRegister(requestData, registerRes);

    response.set_content(registerRes.toJsonString(), "text/plain");
    return 0;
}

int getDeviceList_service_request_handler(const Request& request, Response& response) {
    //todo 向哪个站点请求？


    return 0;
}

int getTvInfo_service_request_handler(const Request& request, Response& response) {

    //todo 向哪个站点请求？

    return 0;
}

int controlDevice_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int radarReportEnable_service_request_handler(const Request& request, Response& response) {

    return 0;
}