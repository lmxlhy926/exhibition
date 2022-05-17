//
// Created by 78472 on 2022/5/17.
//

#include "serviceHandler.h"
#include "qlibc/QData.h"

int deviceList_service_request_handler(const Request& request, Response& response){
    qlibc::QData data;
    data.loadFromFile(R"(D:\bywg\project\exhibition\unit\testSite\configData\devicelist.json)");
    response.set_content(data.toJsonString(), "text/json");

    return 0;
}


int controlDevice_service_request_handler(const Request& request, Response& response){
    std::cout << "controlDevice_service_request_handler: " << request.body << std::endl;

    return 0;
}






