//
// Created by 78472 on 2022/6/2.
//
#include "downCommand.h"
#include "common/httplib.h"
#include <algorithm>
#include "../param.h"

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
   qlibc::QData controlList;
   ssize_t num = deviceList.size();
   for(Json::ArrayIndex i = 0; i < num; ++i){
       qlibc::QData item = deviceList.getArrayElement(i);
       if(match(item)){     //找到匹配项，则根据匹配项构造控制指令
           string sourceSite = item.getString("sourceSite");
           qlibc::QData controlData;
           controlData.setString("device_id", item.getString("device_id"));
           controlData.putData("command_list", buildCommandList(inParams));
           controlData.setString("sourceSite", sourceSite);
           controlList.append(controlData);
       }
   }
   return controlList;
}

bool DownCommandData::match(qlibc::QData &item) {
    string item_room_no = item.getData("location").getString("room_no");
    string item_device_name = item.getString("device_name");
    string item_device_type = item.getString("device_type");
    string sourceSite = item.getString("sourceSite");

    //用区域和设备名字来判定设备
    if(item_room_no == inParams.getString("area") && item_device_type == code){
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
            command.setString("command_id", hump2Underline(key));
            command.putData("command_para", data.getData(key));
            commandList.append(command);
        }
    }
    return commandList;
}

//包含在设备控制命令列表
bool DownCommandData::commandMatch(string &key) {
    for(auto& command: commandMembers){
        if(command == key){
            return true;
        }
    }
    return false;
}

string DownCommandData::hump2Underline(string in) {
    string out;
    for(int i = 0; i < in.size(); ++i)
    {
        if (in[i] <= 'Z' && in[i] >= 'A') {
            if(i == 0){
                out.push_back(in[i] - 'A' + 'a');
            }else{
                out.push_back('_');
                out.push_back(in[i] - 'A' + 'a');
            }
        }else {
            out.push_back(in[i]);
        }
    }
    return out;
}

string DownCommandData::toUpper(string in) {
    transform(in.begin(), in.end(), in.begin(), ::toupper);
    return in;
}



