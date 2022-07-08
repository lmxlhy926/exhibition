//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "formatTrans/lightControlCmd.h"
#include <sstream>
#include <regex>
#include "formatTrans/JsonCmd2Binary.h"
#include "log/Logging.h"

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
void serialSend(unsigned char *buf, int size){
    shared_ptr<BLETelinkDongle> serial = bleConfig::getInstance()->getSerial();
    if(serial != nullptr){
        std::lock_guard<std::mutex> lg(sendMutex);
        if(serial->sendData(buf, static_cast<int>(size))){
            LOG_RED << "===>send success....";
        }
    }
}

void downCmdHandler(qlibc::QData& cmdData){
    unsigned char buf[100]{};
    qlibc::QData controlData = cmdData.getData("request");
    size_t size = bleJsonCmd2Binaray(controlData, buf, 100);

    stringstream ss;
    for(int i = 0; i < size; i++){
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(buf[i]);
        if(i < size -1)
            ss << " ";
    }
    LOG_RED << "==>sendCmd: " << ss.str();

    serialSend(buf, static_cast<int>(size));
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


int BleDevice_command_test_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        string command = requestBody.getData("request").getString("command");
        unsigned char buf[100]{};
        JsonCmd2Binary::BinaryBuf binaryBuf(buf, 100);

        regex sep(" ");
        sregex_token_iterator p(command.cbegin(), command.cend(), sep, -1);
        sregex_token_iterator e;
        for(; p != e; ++p){
            binaryBuf.append(*p);
        }
        LOG_HLIGHT << "--->send: " << command;

        shared_ptr<BLETelinkDongle> serial = bleConfig::getInstance()->getSerial();
        if(serial != nullptr){
            std::lock_guard<std::mutex> lg(sendMutex);
            if(serial->sendData(buf, static_cast<int>(binaryBuf.size()))){
            }
        }

        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }

    return 0;
}

