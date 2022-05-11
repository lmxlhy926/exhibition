//
// Created by 78472 on 2022/5/11.
//

#include "serviceRequestHandler.h"

int sceneListRequest_service_request_handler(const Request& request, Response& response) {
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

int subDeviceRegister_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int domainIdRequest_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int engineer_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int getDeviceList_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int getTvInfo_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int controlDevice_service_request_handler(const Request& request, Response& response) {

    return 0;
}

int radarReportEnable_service_request_handler(const Request& request, Response& response) {

    return 0;
}