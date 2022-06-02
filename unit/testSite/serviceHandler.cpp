//
// Created by 78472 on 2022/5/17.
//

#include "serviceHandler.h"
#include "qlibc/QData.h"
#include "path.h"
#include "qlibc/FileUtils.h"

int deviceList_service_request_handler(const Request& request, Response& response){
    qlibc::QData data;
    std::string path = FileUtils::contactFileName(ProjectPath, R"(unit\paramData\testSite\devicelist.json)");
    data.loadFromFile(path);

    response.set_content(data.toJsonString(), "text/json");
    std::cout << "request device list: " << data.toJsonString() << std::endl;

    return 0;
}


int controlDevice_service_request_handler(const Request& request, Response& response){
    std::cout << "controlDevice_service_request_handler: " << qlibc::QData(request.body).toJsonString(true) << std::endl;

    return 0;
}

int tvMac_service_request_handler(const Request& request, Response& response){
    qlibc::QData data;
    data.loadFromFile(R"(D:\bywg\project\exhibition\unit\paramData\testSite\tvMac.json)");
    response.set_content(data.toJsonString(), "text/json");

    return 0;
}




