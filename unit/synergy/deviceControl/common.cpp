//
// Created by 78472 on 2022/6/2.
//
#include "common.h"
#include "common/httplib.h"

bool ControlBase::match(const DownCommandData &downCommand, qlibc::QData &deviceItem){
    return downCommand.deviceName == deviceItem.getString("nick_name") &&
           downCommand.area == deviceItem.getData("location").getString("room_no");
}

qlibc::QData ControlBase::constructCtrCmd(const DownCommandData &downCommand, qlibc::QData &deviceItem){
    qlibc::QData command;

    if(downCommand.command == "turnOn"){
        command.setString("command_id", "power");
        command.setString("command_para", "on");

    }else if(downCommand.command == "turnOff"){
        command.setString("command_id", "power");
        command.setString("command_para", "off");
    }

    return encapsulate(command, deviceItem);
}

qlibc::QData ControlBase::encapsulate(qlibc::QData& command, qlibc::QData& deviceItem){
    qlibc::QData command_list, deviceControlItem, device_list;
    qlibc::QData requestBody, constructedCommand;

    command_list.append(command);

    deviceControlItem.setString("device_id", deviceItem.getString("device_id"));
    deviceControlItem.putData("command_list", command_list);

    device_list.append(deviceControlItem);

    requestBody.putData("device_list", device_list);

    constructedCommand.setString("service_id", "control_device");
    constructedCommand.putData("request", requestBody);

    return constructedCommand;
}

bool ControlBase::sitePostRequest(const string& ip, int port, qlibc::QData& request, qlibc::QData& response){
    httplib::Client client(ip, port);
    httplib::Result result =  client.Post("/", request.toJsonString(), "text/json");
    if(result != nullptr){
        response.setInitData(qlibc::QData(result.value().body));
        return true;
    }
    return false;
}
