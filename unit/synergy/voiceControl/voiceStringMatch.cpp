//
// Created by 78472 on 2022/12/11.
//

#include "voiceStringMatch.h"
#include "qlibc/QData.h"
#include "../sourceManage/deviceManager.h"
#include "../sourceManage/groupManager.h"
#include "../param.h"
#include "log/Logging.h"
#include "common/httpUtil.h"


string voiceStringMatchControl::lastModifyedDeviceOrGroup;
int voiceStringMatchControl::tempLuminance = 0;
int voiceStringMatchControl::tempTemperature = 0;


voiceStringMatchControl::voiceStringMatchControl(string& ctrlStr)   :
    controlParseString(ctrlStr),
    roomList({
        "主卧",
        "儿童房",
        "书房",

        "客厅",
        "餐厅",
        "厨房",
        "主卫",
        "次卫",

        "玄关",
        "景观阳台",
        "生活阳台",
        "过道",
    }),
    deviceTypeMap({
        {"灯", "LIGHT"}
    }),
    matchRex2ActionCode({
        {".*(打开).*", ActionCode::powerOn},
        {".*(关闭).*", ActionCode::powerOff},
        {".*(亮一点).*",ActionCode::luminanceUp},
        {".*(暗一点).*", ActionCode::luminanceDown},
        {".*(白一点).*", ActionCode::temperatureUp},
        {".*(黄一点).*", ActionCode::temperatureDown},
        {".*((调|变|换|设)(到|成|为|置)).*(亮度).*", ActionCode::luminance1},
        {".*(亮度).*((调|变|换|设)(到|成|为|置)).*", ActionCode::luminance2},
        {".*((调|变|换|设)(到|成|为|置)).*(色温).*", ActionCode::color_temperature1},
        {".*(色温).*((调|变|换|设)(到|成|为|置)).*", ActionCode::color_temperature2}
    }),
    actionCodeCaptureGroup({
        {ActionCode::powerOn, {1}},
        {ActionCode::powerOff, {1}},
        {ActionCode::luminanceUp, {1}},
        {ActionCode::luminanceDown, {1}},
        {ActionCode::temperatureUp, {1}},
        {ActionCode::temperatureDown, {1}},
        {ActionCode::luminance1, {1, 4}},
        {ActionCode::luminance2, {1, 2}},
        {ActionCode::color_temperature1, {1, 4}},
        {ActionCode::color_temperature2, {1, 2}}
    })
{}

void voiceStringMatchControl::parseAndControl() {
    //去除包含的字符
    regex sep("[ ]+");
    sregex_token_iterator end;
    sregex_token_iterator p(controlParseString.cbegin(), controlParseString.cend(), sep, {-1});
    for(; p != end; p++){
        voiceString += p->str();
    }
    LOG_INFO << "total: " << voiceString;
    ParsedItem parsedItem;

    //确定房间，以及是否包含所有
    for(auto& room :roomList){
        smatch sm;
        if(regex_search(voiceString, sm, regex(room))){
            parsedItem.roomName = room;
            voiceString.erase(sm.position(), sm.length());
            break;
        }
    }
    auto allPos = voiceString.find("所有");
    if(allPos != std::string::npos){
        voiceString.erase(allPos, string("所有").length());
        parsedItem.hasAll = true;
    }
    LOG_INFO << "afterLocation: " << voiceString;

    //确定动作
    for(auto& mr2acItem : matchRex2ActionCode){
        smatch sm;
        if(regex_match(voiceString, sm, regex(mr2acItem.first))){
            ActionCode code = mr2acItem.second;
            for(auto& accgItem : actionCodeCaptureGroup){
                if(accgItem.first == code){
                    std::map<string, int> str2DelMap;
                    for(auto& captureGroupIndex : accgItem.second){
                        str2DelMap.insert(std::make_pair(sm.str(captureGroupIndex), sm.length(captureGroupIndex)));
                    }
                    //删除匹配到的子字符串
                    for(auto& elem : str2DelMap){
                        auto pos = voiceString.find(elem.first);
                        if(pos != std::string::npos){
                            voiceString.erase(pos, elem.second);
                        }
                    }
                    parsedItem.actionCode = code;
                    break;
                }
            }
        }
    }
    LOG_INFO << "afterAction: " << voiceString;

    //查找确定的设备、分组
    if(findDeviceIdOrGroupId(voiceString, parsedItem)){
        LOG_INFO << "afterDevice: " << voiceString;
    }else{
        LOG_RED << "vocieString matchResult: can not find device or group, match missed.....";
        return;
    }

    //确定参数
    smatch sm;
    if(regex_match(voiceString, sm, regex("[^0-9]*(\\d*)[^0-9]*"))){
        parsedItem.param = sm.str(1);
        voiceString.erase(sm.position(1), sm.length(1));
    }
    LOG_INFO << "afterParam: " << voiceString;

    printParsedItem(parsedItem);

    //控制设备
    controlByParsedItem(parsedItem);
}

string voiceStringMatchControl::code2Action(ActionCode code){
    string action;
    if(code == ActionCode::NoneAction){
        action = "NoneAction";
    }else if(code == ActionCode::powerOn){
        action = "powerOn";
    }else if(code == ActionCode::powerOff){
        action = "powerOff";
    }else if (code == ActionCode::luminance1 || code == ActionCode::luminance2){
        action = "set luminance";
    }else if(code == ActionCode::color_temperature1 || code == ActionCode::color_temperature2){
        action = "set color_temperature";
    }else if(code == ActionCode::luminanceUp){
        action = "luminance up";
    }else if(code == ActionCode::luminanceDown){
        action = "luminance down";
    }else if(code == ActionCode::temperatureUp){
        action = "temperature up";
    }else if(code == ActionCode::temperatureDown){
        action = "temperature down";
    }
    return action;
}

void voiceStringMatchControl::printParsedItem(struct ParsedItem& item){
    qlibc::QData data;
    data.setString("roomName", item.roomName);
    data.setString("action", code2Action(item.actionCode));
    if(item.ctrlType == ControlType::Device){
        data.setString("deviceId", item.devIdGrpId);
    }else if(item.ctrlType == ControlType::Group){
        data.setString("groupId", item.devIdGrpId);
    }else if(item.ctrlType == ControlType::Type){
        data.setString("type", item.devIdGrpId);
    }
    data.setString("deviceType", item.deviceType);
    data.setBool("hasAll", item.hasAll);
    data.setValue("param", item.param);
    data.setString("sourceSite", item.sourceSite);

    LOG_PURPLE << "parseItem: " << data.toJsonString(true);
}

void voiceStringMatchControl::action2Command(ParsedItem& parsedItem, CommandItem& commandItem){
    std::lock_guard<std::mutex> lg(Mutex);

    if(parsedItem.actionCode == ActionCode::powerOn){   //开
        commandItem.command_id = "power";
        commandItem.command_para = "on";
        lastModifyedDeviceOrGroup = parsedItem.devIdGrpId;

    }else if(parsedItem.actionCode == ActionCode::powerOff){    //关
        commandItem.command_id = "power";
        commandItem.command_para = "off";
        lastModifyedDeviceOrGroup = parsedItem.devIdGrpId;

    }else if(parsedItem.actionCode == ActionCode::luminance1 || parsedItem.actionCode == ActionCode::luminance2){   //设置亮度
        commandItem.command_id = "luminance";
        int luminanceValue = static_cast<int>(atoi(parsedItem.param.c_str()) * 2.55);
        commandItem.command_para = luminanceValue;
        lastModifyedDeviceOrGroup = parsedItem.devIdGrpId;

    }else if(parsedItem.actionCode == ActionCode::color_temperature1 || parsedItem.actionCode == ActionCode::color_temperature2){   //设置色温
        commandItem.command_id = "color_temperature";
        int ctValue = static_cast<int>(atoi(parsedItem.param.c_str()) * 38 + 2700);
        commandItem.command_para = ctValue;
        lastModifyedDeviceOrGroup = parsedItem.devIdGrpId;

    }else if(parsedItem.actionCode == ActionCode::luminanceUp || parsedItem.actionCode == ActionCode::luminanceDown){   //亮一点、暗一点
        commandItem.command_id = "luminance";
        if(lastModifyedDeviceOrGroup != parsedItem.devIdGrpId){
            tempLuminance = defaultLuminance;
        }else{
            if(parsedItem.actionCode == ActionCode::luminanceUp){
                if(tempLuminance + deltaLuminance > 100){
                    tempLuminance = 100;
                }else{
                    tempLuminance += deltaLuminance;
                }
            }else{
                if(tempLuminance - deltaLuminance < 0){
                    tempLuminance = 0;
                }else{
                    tempLuminance -= deltaLuminance;
                }
            }
        }
        int valueInt = static_cast<int>(tempLuminance * 2.55);
        commandItem.command_para = valueInt;
        lastModifyedDeviceOrGroup = parsedItem.devIdGrpId;

    }else if(parsedItem.actionCode == ActionCode::temperatureUp || parsedItem.actionCode == ActionCode::temperatureDown){   //冷一点、暖一点
        commandItem.command_id = "color_temperature";
        if(lastModifyedDeviceOrGroup != parsedItem.devIdGrpId){
            tempTemperature = defaultTemperature;
        }else{
            if(parsedItem.actionCode == ActionCode::temperatureUp){
                if(tempTemperature + deltaTemperature > 100){
                    tempTemperature = 100;
                }else{
                    tempTemperature += deltaTemperature;
                }
            }else{
                if(tempTemperature - deltaTemperature < 0){
                    tempTemperature = 0;
                }else{
                    tempTemperature -= deltaTemperature;
                }
            }
        }
        int valueInt = static_cast<int>(tempTemperature * 38 + 2700);
        commandItem.command_para = valueInt;
        lastModifyedDeviceOrGroup = parsedItem.devIdGrpId;
    }

}

bool voiceStringMatchControl::getSpecificDeviceId(qlibc::QData& deviceList, string& str, string& deviceId, string& sourceSite) {
    Json::ArrayIndex size = deviceList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string device_name = item.getString("device_name");
        if(!str.empty() && !device_name.empty()){
            smatch sm;
            if(regex_search(str, sm, regex(device_name))){
                deviceId = item.getString("device_id");
                sourceSite = item.getString("sourceSite");
                str.erase(sm.position(), sm.length());
                return true;
            }
        }
    }
    return false;
}


bool voiceStringMatchControl::getSpecificGroupId(qlibc::QData& groupList, string& str, string& groupId, string& sourceSite) {
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string group_name = item.getString("group_name");
        if(!str.empty() && !group_name.empty()){
            smatch sm;
            if(regex_search(str, sm,regex(group_name))){
                groupId = item.getString("group_id");
                sourceSite = item.getString("sourceSite");
                str.erase(sm.position(), sm.length());
                return true;
            }
        }
    }
    return false;
}

bool voiceStringMatchControl::getDeviceType(string& str, string& deviceType){
    for(auto& type : deviceTypeMap){
        if(!str.empty() && !type.first.empty()){
            smatch sm;
            if(regex_search(str, sm, regex(type.first))){
                deviceType = type.second;
                str.erase(sm.position(), sm.length());
                return true;
            }
        }
    }
    return false;
}

bool voiceStringMatchControl::getGroupIdFromRoomNameAndType(qlibc::QData& groupList, string& roomName, string& deviceType, string& groupId, string& sourceSite) {
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string room_name = item.getData("location").getString("room_name");
        string group_type = item.getString("group_type");
        if(room_name == roomName){
            groupId = item.getString("group_id");
            sourceSite = item.getString("sourceSite");
            return true;
        }
    }
    return false;
}

bool voiceStringMatchControl::findDeviceIdOrGroupId(string& str, ParsedItem& parsedItem){
    qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
    qlibc::QData groupList = GroupManager::getInstance()->getAllGroupList();
    string deviceIdOrGroupId, sourceSite;
    string deviceType;
    bool isMatch = false;

    if(getSpecificDeviceId(deviceList, str, deviceIdOrGroupId, sourceSite)){   //具体设备名称
        parsedItem.ctrlType = ControlType::Device;
        parsedItem.devIdGrpId = deviceIdOrGroupId;
        parsedItem.sourceSite = sourceSite;
        isMatch = true;

    }else if(getSpecificGroupId(groupList, str, deviceIdOrGroupId, sourceSite)){ //具体组ID
        parsedItem.ctrlType = ControlType::Group;
        parsedItem.devIdGrpId = deviceIdOrGroupId;
        parsedItem.sourceSite = sourceSite;
        isMatch = true;

    }else if(getDeviceType(str, deviceType)){   //<房间 + 类型> 确定组名
        if(!parsedItem.roomName.empty()){
            getGroupIdFromRoomNameAndType(groupList, parsedItem.roomName, deviceType, deviceIdOrGroupId, sourceSite);
            parsedItem.ctrlType = ControlType::Group;
            parsedItem.devIdGrpId = deviceIdOrGroupId;
            parsedItem.sourceSite = sourceSite;
            isMatch = true;
        }else if(parsedItem.hasAll){    //发送到所有的蓝牙站点
            parsedItem.ctrlType = ControlType::Group;
            parsedItem.devIdGrpId = "FFFF";
            parsedItem.sourceSite = "ALL";
            isMatch = true;
        }
    }

    return isMatch;
}

void voiceStringMatchControl::controlByParsedItem(ParsedItem& parsedItem){
    qlibc::QData requestData, responseData;
    if(parsedItem.ctrlType == ControlType::Device){
        Json::Value command, commandList(Json::arrayValue), deviceItem, deviceList(Json::arrayValue);
        CommandItem commandItem;
        action2Command(parsedItem, commandItem);
        command["command_id"] = commandItem.command_id;
        command["command_para"] = commandItem.command_para;
        commandList.append(command);
        deviceItem["device_id"] = parsedItem.devIdGrpId;
        deviceItem["command_list"] = commandList;
        deviceList.append(deviceItem);

        requestData.setString("service_id", "control_device");
        requestData.putData("request", qlibc::QData().setValue("device_list", deviceList));
        LOG_GREEN << "controlRequest: " << requestData.toJsonString();
        if(parsedItem.sourceSite != "ALL"){
            SiteRecord::getInstance()->sendRequest2Site(parsedItem.sourceSite, requestData, responseData);
            LOG_BLUE << parsedItem.sourceSite << " controlResponse: " << responseData.toJsonString();
        }else{
            std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
            smatch sm;
            for(auto& elem : siteNameSet){
                if(regex_match(elem, sm, regex("(.*):(.*)"))){
                    string ip = sm.str(1);
                    string siteID = sm.str(2);
                    if(siteID == BleSiteID){
                        SiteRecord::getInstance()->sendRequest2Site(sm.str(0), requestData, responseData);
                        LOG_BLUE << sm.str(0) << " controlResponse: " << responseData.toJsonString();
                    }
                }
            }
        }

    }else if(parsedItem.ctrlType == ControlType::Group){
        Json::Value command, commandList(Json::arrayValue), groupItem, group_list(Json::arrayValue);
        CommandItem commandItem;
        action2Command(parsedItem, commandItem);
        command["command_id"] = commandItem.command_id;
        command["command_para"] = commandItem.command_para;
        commandList.append(command);
        groupItem["group_id"] = parsedItem.devIdGrpId;
        groupItem["command_list"] = commandList;
        group_list.append(groupItem);

        requestData.setString("service_id", "control_group");
        requestData.putData("request", qlibc::QData().setValue("group_list", group_list));
        LOG_GREEN << "controlRequest: " << requestData.toJsonString();
        if(parsedItem.sourceSite != "ALL"){
            SiteRecord::getInstance()->sendRequest2Site(parsedItem.sourceSite, requestData, responseData);
            LOG_BLUE << parsedItem.sourceSite << " controlResponse: " << responseData.toJsonString();
        }else{
            std::set<string> siteNameSet = SiteRecord::getInstance()->getSiteName();
            smatch sm;
            for(auto& elem : siteNameSet){
                if(regex_match(elem, sm, regex("(.*):(.*)"))){
                    string ip = sm.str(1);
                    string siteID = sm.str(2);
                    if(siteID == BleSiteID){
                        SiteRecord::getInstance()->sendRequest2Site(sm.str(0), requestData, responseData);
                        LOG_BLUE <<  sm.str(0) << " controlResponse: " << responseData.toJsonString();
                    }
                }
            }
        }
    }

}

