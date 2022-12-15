//
// Created by 78472 on 2022/12/11.
//

#include "voiceStringMatch.h"
#include "qlibc/QData.h"
#include "../deviceGroupManage/deviceManager.h"
#include "../deviceGroupManage/groupManager.h"
#include "common/httpUtil.h"
#include <regex>


//开关灯
regex voice_expr_power_on("(.*)(灯)(.*)(开)(.*)");
regex voice_expr_power_on_1("(.*)(开)(.*)(灯)(.*)");
regex voice_expr_power_off("(.*)(灯)(.*)(关)(.*)");
regex voice_expr_power_off_1("(.*)(关)(.*)(灯)(.*)");


void voiceStringMatchControl::parseAndControl() {
    smatch sm;
    //开关指令
    if(regex_match(controlString, sm, voice_expr_power_on) || regex_match(controlString, sm, voice_expr_power_on_1)){
        MatchItem matchItem;
        matchItem.command_id = "power";
        matchItem.command_para = "on";
        parse2MatchItem(matchItem);
        controlDeviceOrGroupByMatchItem(matchItem);

    }else if(regex_match(controlString, sm, voice_expr_power_off) || regex_match(controlString, sm, voice_expr_power_off_1)){
        MatchItem matchItem;
        matchItem.command_id = "power";
        matchItem.command_para = "off";
        parse2MatchItem(matchItem);
        controlDeviceOrGroupByMatchItem(matchItem);
    }
}


bool voiceStringMatchControl::getSpecificDeviceId(string parseStr, string &deviceId, string& sourceSite, string& siteIp) {
    qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
    Json::ArrayIndex size = deviceList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        if(regex_search(parseStr, regex(item.getString("device_name")))){
            deviceId = item.getString("device_id");
            sourceSite = item.getString("sourceSite");
            return true;
        }
    }
    return false;
}


bool voiceStringMatchControl::getSpecificGroupId(string parseStr, string &groupId, string& sourceSite, string& siteIp) {
    qlibc::QData groupList = GroupManager::getInstance()->getAllGroupList();
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        if(regex_search(parseStr, regex(item.getString("group_name")))){
            groupId = item.getString("group_id");
            sourceSite = item.getString("sourceSite");
            return true;
        }
    }
    return false;
}


bool voiceStringMatchControl::getGroupIdFromRoomName(string parseStr, string &groupId, string& sourceSite, string& siteIp) {
    qlibc::QData groupList = GroupManager::getInstance()->getAllGroupList();
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        if(regex_search(parseStr, regex(item.getData("location").getString("room_name")))){
            groupId = item.getString("group_id");
            sourceSite = item.getString("sourceSite");
            return true;
        }
    }
    return false;
}


bool voiceStringMatchControl::getFullGroupId(string parseStr, string &groupId, string& sourceSite, string& siteIp) {
    if(regex_search(parseStr, regex("所有"))){
        groupId = "FFFF";
        sourceSite = "ble_site";
        return true;
    }
    return false;
}

bool voiceStringMatchControl::parse2MatchItem(MatchItem &matchItem) {
    string deviceIdOrGroupId, sourceSite, siteIp;
    smatch sm;
    bool isMatch = false;

    for(int i = 1; i < sm.size(); ++i){
        if(getSpecificDeviceId(sm[i].str(), deviceIdOrGroupId, sourceSite, siteIp)){            //具体设备名称
            matchItem.type = VoiceMatchType::device;
            matchItem.id = deviceIdOrGroupId;
            matchItem.sourceSite = sourceSite;
            isMatch = true;
            break;

        }else if(getSpecificGroupId(sm[i].str(), deviceIdOrGroupId, sourceSite, siteIp)){       //具体组ID
            matchItem.type = VoiceMatchType::group;
            matchItem.id = deviceIdOrGroupId;
            matchItem.sourceSite = sourceSite;
            isMatch = true;
            break;

        }else if(getGroupIdFromRoomName(sm[i].str(), deviceIdOrGroupId, sourceSite, siteIp)){   //房间默认组ID
            matchItem.type = VoiceMatchType::group;
            matchItem.id = deviceIdOrGroupId;
            matchItem.sourceSite = sourceSite;
            isMatch = true;
            break;

        }else if(getFullGroupId(sm[i].str(), deviceIdOrGroupId, sourceSite, siteIp)){           //所有
            matchItem.type = VoiceMatchType::group;
            matchItem.id = "FFFF";
            matchItem.sourceSite = sourceSite;
            isMatch = true;
            break;
        }
    }

    if(isMatch){
        return true;
    }

    return false;
}


void voiceStringMatchControl::controlDeviceOrGroupByMatchItem(MatchItem &matchItem) {
    qlibc::QData requestData, responseData;
    if(matchItem.type == VoiceMatchType::device){
        Json::Value command, commandList(Json::arrayValue), deviceItem, deviceList(Json::arrayValue);
        command["command_id"] = matchItem.command_id;
        command["command_para"] = matchItem.command_para;
        commandList.append(command);
        deviceItem["device_id"] = matchItem.id;
        deviceItem["command_list"] = commandList;
        deviceList.append(deviceItem);

        requestData.setString("service_id", "control_device");
        requestData.putData("request", qlibc::QData().setValue("device_list", deviceList));
        SiteRecord::getInstance()->sendRequest2Site(matchItem.sourceSite, requestData, responseData);

    }else if(matchItem.type == VoiceMatchType::group){
        Json::Value command, commandList(Json::arrayValue), groupItem, group_list(Json::arrayValue);
        command["command_id"] = matchItem.command_id;
        command["command_para"] = matchItem.command_para;
        commandList.append(command);
        groupItem["group_id"] = matchItem.id;
        groupItem["command_list"] = commandList;
        group_list.append(groupItem);

        requestData.setString("service_id", "control_group");
        requestData.putData("request", qlibc::QData().setValue("group_list", group_list));
        SiteRecord::getInstance()->sendRequest2Site(matchItem.sourceSite, requestData, responseData);
    }
}
