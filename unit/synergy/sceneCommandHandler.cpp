//
// Created by 78472 on 2022/5/17.
//

#include "sceneCommandHandler.h"
#include <iostream>
#include "socket/httplib.h"
#include "paramConfig.h"

bool sitePostRequest(const string& ip, int port, qlibc::QData& request, qlibc::QData& response){
    httplib::Client client(ip, port);
    httplib::Result result =  client.Post("/", request.toJsonString(), "text/json");
    if(result != nullptr){
        response.setInitData(qlibc::QData(result.value().body));
        return true;
    }
    return false;
}


qlibc::QData construct_lightControlCommand(const struct controlData& controlCommand, qlibc::QData& data){

    Json::Value value;
    value["t"] = "int";
    if(controlCommand.command == "turnOn"){
        value["v"] = 1;
    }else{
        value["v"] = 0;
    }

    qlibc::QData command;
    command.setString("device_id", data.getString("device_id"));
    command.setString("device_brand", data.getString("device_brand"));
    command.setString("device_source", data.getString("device_source"));
    command.setString("device_subid", data.getString("sub_device_id"));
    command.setString("device_type", data.getString("device_type"));
    command.setString("stream_id", "devstate");
    command.setBool("absolute", true);
    command.setValue("value", value);

    qlibc::QData device_list;
    device_list.append(command);

    qlibc::QData request;
    request.putData("device_list", device_list);

    qlibc::QData retData;
    retData.setString("service_id", "control_device");
    retData.putData("request", request);


    return retData;
}


bool deviceControlHandler(const string& uri, qlibc::QData& message){
    std::cout << "received: <" << uri << ">---" << message.toJsonString() << std::endl;

    const struct controlData controlCommand(message);
    qlibc::QData request, response;
    request.setString("service_id", "get_device_list");
    request.setValue("request", Json::nullValue);

    bool ret = sitePostRequest(ADAPTER_IP, ADAPTER_PORT,
                                             request, response);

    if(ret){
        if(response.getInt("code") == 0 && controlCommand.deviceType == "light"){
            qlibc::QData deviceList = response.getData("response").getData("device_list");
            for(int i = 0 ; i < deviceList.size(); i++){
                qlibc::QData ithData = deviceList.getArrayElement(i);
                if(ithData.getString("nick_name") == controlCommand.deviceName){
                    qlibc::QData commandData = construct_lightControlCommand(controlCommand, ithData);
                    qlibc::QData controlRet;
                    sitePostRequest(ADAPTER_IP, ADAPTER_PORT, commandData, controlRet);
                    break;
                }
            }
        }
    }

    return true;
}







