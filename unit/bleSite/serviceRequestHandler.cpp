//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "formatTrans/downCmd.h"
#include <sstream>

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

static std::mutex sendMutex;

void downCmdHandler(qlibc::QData& cmdData){
    unsigned char buf[100]{};
    size_t size = bleJsonCmd2Binaray(cmdData, buf, 100);

    stringstream ss;
    for(int i = 0; i < size; i++){
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(buf[i]);
        if(i < size -1)
            ss << " ";
    }
    std::cout << "==>sendCmd: " << ss.str() << std::endl;

    shared_ptr<BLETelinkDongle> serial = bleConfig::getInstance()->getSerial();
    if(serial != nullptr){
        std::lock_guard<std::mutex> lg(sendMutex);
        if(serial->sendData(buf, static_cast<int>(size))){
            printf("===>send success....\n");
        }
    }
}


int BleDevice_command_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        downCmdHandler(requestBody);
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


