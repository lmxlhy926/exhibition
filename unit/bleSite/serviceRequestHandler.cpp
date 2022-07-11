//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "formatTrans/lightControlCmd.h"
#include <regex>
#include "log/Logging.h"
#include "logic/logicControl.h"
#include "formatTrans/lightControlCmd.h"

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

void downCmdHandler(qlibc::QData& request){
    qlibc::QData cmdData = request.getData("request");
    LogicControl::parse(cmdData);
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

        serialSend(buf, static_cast<int>(binaryBuf.size()));

        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }

    return 0;
}

