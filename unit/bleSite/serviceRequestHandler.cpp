//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "formatTrans/downBinaryCmd.h"
#include <regex>
#include "log/Logging.h"
#include "logic/logicControl.h"


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


int BleDevice_command_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData cmdData = requestBody.getData("request");
            lc.parse(cmdData);
        });
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
        BinaryBuf binaryBuf(buf, 100);

        regex sep(" ");
        sregex_token_iterator p(command.cbegin(), command.cend(), sep, -1);
        sregex_token_iterator e;
        for(; p != e; ++p){
            binaryBuf.append(*p);
        }
        LOG_HLIGHT << "--->send: " << command;

        DownBinaryUtil::serialSend(buf, static_cast<int>(binaryBuf.size()));

        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }

    return 0;
}

