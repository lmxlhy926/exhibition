//
// Created by 78472 on 2022/6/2.
//
#include "downCommand.h"
#include "http/httplib.h"
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
    LOG_INFO << "controlList: " << controlList.toJsonString();
    return controlList;
}

bool DownCommandData::fuzzyMatch(qlibc::QData& item) {
    //设备属性提取
    string item_nick_name = item.getString("nick_name");
    string item_device_type = item.getString("device_type");
    string item_room_no = item.getData("location").getString("room_no");

    //匹配参数提取
    string area = inParams.getString("area");   //区域
    string kind = inParams.getString("kind");   //zd, fd, all

    //如果是灯设备
    if(code == "LIGHT" && (item_device_type == "LIGHT_SWITCH" || item_device_type == "LIGHT")){  //类型匹配
        if(area == "all" || area == item_room_no){      //区域匹配
            if(kind == "all" || kind.empty()){
                return true;
            }else if(kind == item_nick_name){   //类型匹配
                return true;
            }else{
                return false;
            }
        }
    }

    //其它设备
    if(code == item_device_type){   //类型匹配
        if(area == "all" || area == item_room_no){  //类型匹配
            return true;
        }else{
            return false;
        }
    }

    return false;
}


bool DownCommandData::match(qlibc::QData& item){
    string deviceCode = inParams.getString("deviceCode");
    string item_device_id = item.getString("device_id");
    if(!deviceCode.empty()){    //通过设备标识码来判断
        if(item_device_id == deviceCode){
            return true;
        }else{
            return false;
        }

    }else{  //通过区域、类型来判断
        return fuzzyMatch(item);
    }
}

qlibc::QData DownCommandData::buildCommandList(qlibc::QData& data){
    qlibc::QData commandList;
    Json::Value::Members keys = data.getMemberNames();
    for(auto& key : keys){
        if(commandMatch(key)){  //inParams包含控制命令
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



