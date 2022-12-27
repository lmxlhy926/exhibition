//
// Created by 78472 on 2022/12/11.
//

#include "voiceStringMatch.h"
#include "qlibc/QData.h"
#include "../deviceGroupManage/deviceManager.h"
#include "../deviceGroupManage/groupManager.h"
#include "../param.h"
#include "log/Logging.h"
#include "common/httpUtil.h"

//开关灯
//regex voice_expr_power_on("(.*)(灯)(.*)(开)(.*)");
//regex voice_expr_power_on_1("(.*)(开)(.*)(灯)(.*)");
//regex voice_expr_power_off("(.*)(灯)(.*)(关)(.*)");
//regex voice_expr_power_off_1("(.*)(关)(.*)(灯)(.*)");

regex voice_expr_power_on("(.*)(开)(.*)");
regex voice_expr_power_off("(.*)(关)(.*)");



void voiceStringMatchControl::parseAndControl() {
    smatch sm;
    //开关指令
    if(regex_match(controlString, sm, voice_expr_power_on)){
        LOG_GREEN << "voice_expr_power_on: " << "<" << sm.str(1) << ">" << sm.str(2) << "<" << sm.str(3) << ">";
        MatchItem matchItem{};
        matchItem.command_id = "power";
        matchItem.command_para = "on";
        parse2MatchItem(sm, matchItem);
        controlDeviceOrGroupByMatchItem(matchItem);

    }else if(regex_match(controlString, sm, voice_expr_power_off)){
        MatchItem matchItem{};
        matchItem.command_id = "power";
        matchItem.command_para = "off";
        parse2MatchItem(sm, matchItem);
        controlDeviceOrGroupByMatchItem(matchItem);
    }
}


bool voiceStringMatchControl::getSpecificDeviceId(qlibc::QData& deviceList, string parseStr, string& deviceId, string& sourceSite) {
    Json::ArrayIndex size = deviceList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string device_name = item.getString("device_name");
        if(!parseStr.empty() && !device_name.empty()){
            if(regex_search(parseStr, regex(device_name))){
                LOG_INFO << "matchDevice: " << item.toJsonString(true);
                deviceId = item.getString("device_id");
                sourceSite = item.getString("sourceSite");
                return true;
            }
        }
    }
    return false;
}


bool voiceStringMatchControl::getSpecificGroupId(qlibc::QData& groupList, string parseStr, string& groupId, string& sourceSite) {
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string group_name = item.getString("group_name");
        if(!parseStr.empty() && !group_name.empty()){
            if(regex_search(parseStr, regex(group_name))){
                qlibc::QData matchGroup;
                matchGroup.setString("group_id", item.getString("group_id"));
                matchGroup.setString("group_name", item.getString("group_name"));
                matchGroup.putData("location", item.getData("location"));
                matchGroup.setString("sourceSite", item.getString("sourceSite"));
                LOG_INFO << "matchGroup: " << matchGroup.toJsonString(true);
                groupId = item.getString("group_id");
                sourceSite = item.getString("sourceSite");
                return true;
            }
        }
    }
    return false;
}


bool voiceStringMatchControl::getGroupIdFromRoomName(qlibc::QData& groupList, string parseStr, string& groupId, string& sourceSite) {
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string room_name = item.getData("location").getString("room_name");
        if(!parseStr.empty() && !room_name.empty()){
            if(regex_search(parseStr, regex(room_name))){
                qlibc::QData matchGroup;
                matchGroup.setString("group_id", item.getString("group_id"));
                matchGroup.setString("group_name", item.getString("group_name"));
                matchGroup.putData("location", item.getData("location"));
                matchGroup.setString("sourceSite", item.getString("sourceSite"));
                LOG_INFO << "matchGroup: " << matchGroup.toJsonString(true);
                groupId = item.getString("group_id");
                sourceSite = item.getString("sourceSite");
                return true;
            }
        }
    }
    return false;
}


bool voiceStringMatchControl::getFullGroupId(string parseStr, string &groupId, string& sourceSite) {
    if(regex_search(parseStr, regex("所有"))){
        groupId = "FFFF";
        sourceSite = "ble_site";
        return true;
    }
    return false;
}

void voiceStringMatchControl::printMatchItem(MatchItem& matchItem){
    string type = "wrongType";
    if(matchItem.type == VoiceMatchType::Device){
        type = "device";
    }else if(matchItem.type == VoiceMatchType::Group){
        type = "group";
    }
    qlibc::QData data;
    data.setString("type", type);
    data.setString("id", matchItem.id);
    data.setString("command_id", matchItem.command_id);
    data.setString("command_para", matchItem.command_para);
    data.setString("sourceSite", matchItem.sourceSite);
    LOG_GREEN << "matchItem: " << data.toJsonString(true);
}

bool voiceStringMatchControl::parse2MatchItem(smatch& sm, MatchItem &matchItem) {
    qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
    qlibc::QData groupList = GroupManager::getInstance()->getAllGroupList();
    string deviceIdOrGroupId, sourceSite;
    bool isMatch = false;

    for(int i = 1; i < sm.size(); ++i){
        if(getSpecificDeviceId(deviceList, sm[i].str(), deviceIdOrGroupId, sourceSite)){            //具体设备名称
            matchItem.type = VoiceMatchType::Device;
            matchItem.id = deviceIdOrGroupId;
            matchItem.sourceSite = sourceSite;
            isMatch = true;
            break;

        }else if(getSpecificGroupId(groupList, sm[i].str(), deviceIdOrGroupId, sourceSite)){       //具体组ID
            matchItem.type = VoiceMatchType::Group;
            matchItem.id = deviceIdOrGroupId;
            matchItem.sourceSite = sourceSite;
            isMatch = true;
            break;

        }else if(getGroupIdFromRoomName(groupList, sm[i].str(), deviceIdOrGroupId, sourceSite)){   //房间默认组ID
            matchItem.type = VoiceMatchType::Group;
            matchItem.id = deviceIdOrGroupId;
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
    printMatchItem(matchItem);
    qlibc::QData requestData, responseData;
    if(matchItem.type == VoiceMatchType::Device){
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
        LOG_BLUE << "controlResponse: " << responseData.toJsonString();

    }else if(matchItem.type == VoiceMatchType::Group){
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
        LOG_BLUE << "controlResponse: " << responseData.toJsonString();

    }

//    else if(matchItem.type == VoiceMatchType::All){
//        Json::Value command, commandList(Json::arrayValue), groupItem, group_list(Json::arrayValue);
//        command["command_id"] = matchItem.command_id;
//        command["command_para"] = matchItem.command_para;
//        commandList.append(command);
//        groupItem["group_id"] = matchItem.id;
//        groupItem["command_list"] = commandList;
//        group_list.append(groupItem);
//
//        requestData.setString("service_id", "control_group");
//        requestData.putData("request", qlibc::QData().setValue("group_list", group_list));
//
//        std::set<string> siteNames = SiteRecord::getInstance()->getSiteName();
//        for(auto& elem : siteNames){
//            if(elem == BleSiteID){
//                SiteRecord::getInstance()->sendRequest2Site(elem, requestData, responseData);
//                LOG_BLUE << "controlResponse: " << responseData.toJsonString();
//            }
//        }
//    }

}
