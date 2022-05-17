//
// Created by 78472 on 2022/5/17.
//

#include "sceneCommandHandler.h"
#include <iostream>
#include "socket/httplib.h"

bool sitePostRequest(const string& ip, int port, qlibc::QData& request, qlibc::QData& response){
    httplib::Client client(ip, port);
    httplib::Result result =  client.Post("/", request.toJsonString(), "text/json");
    if(result != nullptr){
        response.setInitData(qlibc::QData(result.value().body));
        return true;
    }
    return false;
}


qlibc::QData construct(const struct controlData& controlCommand, qlibc::QData& data){

}


bool deviceControlHandler(const string& uri, qlibc::QData& message){
    std::cout << "received: <" << uri << ">---" << message.toJsonString() << std::endl;

    const struct controlData controlCommand(message);
    qlibc::QData request, response;
    request.setString("service_id", "get_device_list");
    request.setValue("request", Json::nullValue);

    bool ret = sitePostRequest("127.0.0.1", 60003,
                                             request, response);
    if(ret){
        if(response.getInt("code") == 0 && controlCommand.deviceType == "light"){
            qlibc::QData deviceList = response.getData("response").getData("device_list");
            for(int i = 0 ; i < deviceList.size(); i++){
                qlibc::QData ithData = deviceList.getArrayElement(i);
                if(ithData.getString("nick_name") == controlCommand.deviceName){
                    qlibc::QData postData = construct(controlCommand, ithData);
                    //发送控制请求

                    break;
                }
            }
        }
    }

    return true;
}







