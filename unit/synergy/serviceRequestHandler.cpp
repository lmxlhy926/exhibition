//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"

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

static const nlohmann::json tvSoundMessage = {
        {"funcName", "deviceDataReport"},
        {"deviceType", "tvSound"},
        {"area", ""},
        {"deviceName", ""},
        {"eventName", ""},
        {"params",{
            "sound", "我是长虹小白"
        }}
};

void publish2Client(socketClient& client, const Request& request, Response& response){
    qlibc::QData data(request.body);
    if(data.type() != Json::nullValue){
        std::cout << "Received message to activeApp: " << data.toJsonString() << std::endl;
        client.sendMessage(data.getData("request").toJsonString(), true);
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
}

int tvupload_service_handler(socketClient& client, const Request& request, Response& response){
    publish2Client(client, request, response);
    return 0;
}

int sensor_service_handler(socketClient& client, const Request& request, Response& response){
    publish2Client(client, request, response);
    return 0;
}


int tvSound_service_handler(socketClient& client, const Request& request, Response& response){
    std::cout << "Received message to activeApp: " << request.body << std::endl;
    client.sendMessage(tvSoundMessage.dump(), true);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


int commonEvent_service_handler(socketClient& client, const Request& request, Response& response){
    publish2Client(client, request, response);
    return 0;
}






