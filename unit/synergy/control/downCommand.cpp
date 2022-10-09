//
// Created by 78472 on 2022/6/2.
//
#include "downCommand.h"
#include "common/httplib.h"

/*
 * 灯控
 * 窗帘
 * 空调
 * 空气净化器
 * 加湿器
 */
std::vector<string> commandMembers = {
        "power", "luminance", "color_temperature",
        "open",
        "power", "mode", "temperature", "velocity", "horizontal_wind", "vertical_wind", "ptc_heat", "air_clean", "sleep",
        "power", "auto", "sleep", "anion", "count_down", "cancel_count_down", "velocity", "air_quality",
        "power", "level"
};

qlibc::QData DownCommandData::getContorlData(qlibc::QData &deviceList) {
   ssize_t num = deviceList.size();
   for(Json::ArrayIndex i = 0; i < num; ++i){
       qlibc::QData item = deviceList.getArrayElement(i);
       if(match(item)){
           qlibc::QData controlData;
           controlData.setString("device_id", item.getString("device_id"));
           controlData.putData("command_list", buildCommandList(inParams));
           controlData.setString("sourceSite", item.getString("sourceSite"));
           return controlData;
       }
   }
   return qlibc::QData();
}

bool DownCommandData::match(qlibc::QData &item) {
    string item_room_no = item.getData("location").getString("room_no");
    string item_device_name = item.getString("device_name");
    if(item_room_no == inParams.getString("area") && item_device_name == inParams.getString("productNickname")){
        return true;
    }
    return false;
}

qlibc::QData DownCommandData::buildCommandList(qlibc::QData& data){
    qlibc::QData commandList;
    Json::Value::Members keys = data.getMemberNames();
    for(auto& key : keys){
        if(commandMatch(key)){
            qlibc::QData command;
            command.setString("command_id", key);
            command.putData("command_para", data.getData(key));
            commandList.append(command);
        }
    }
    return commandList;
}

bool DownCommandData::commandMatch(string &key) {
    for(auto& command: commandMembers){
        if(command == key){
            return true;
        }
    }
    return false;
}



