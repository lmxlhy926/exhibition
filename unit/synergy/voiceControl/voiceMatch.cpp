
#include <set>
#include <map>
#include "voiceMatch.h"
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "common/httpUtil.h"
#include "../param.h"
#include "../sourceManage/deviceManager.h"
#include "../sourceManage/groupManager.h"

static std::vector<string> roomList{
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
};

std::map<string, string> deviceTypeMap{
    {"灯", "LIGHT"},
    {"窗帘", "WINDOW_COVERING_SWITCH"}
};

std::map<string, ActionCode> matchRex2ActionCode{
    {".*(打开).*",    ActionCode::powerOn},
    {".*(开).*",      ActionCode::powerOn},
    {".*(关闭).*",    ActionCode::powerOff},
    {".*(关).*",      ActionCode::powerOff},
    {".*(亮一点).*",  ActionCode::luminanceUp},
    {".*(暗一点).*",  ActionCode::luminanceDown},
    {".*(白一点).*",  ActionCode::temperatureUp},
    {".*(黄一点).*",  ActionCode::temperatureDown},
    {".*((调|变|换|设)(到|成|为|置)).*(亮度).*", ActionCode::luminance1},
    {".*(亮度).*((调|变|换|设)(到|成|为|置)).*", ActionCode::luminance2},
    {".*((调|变|换|设)(到|成|为|置)).*(色温).*", ActionCode::color_temperature1},
    {".*(色温).*((调|变|换|设)(到|成|为|置)).*", ActionCode::color_temperature2}
};

std::map<ActionCode, std::vector<int>> actionCodeCaptureGroup{
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
};

std::map<ActionCode, string> actionCode2StringMap{
    {ActionCode::powerOn,               "powerOn"},
    {ActionCode::powerOff,              "powerOff"},
    {ActionCode::luminanceUp,           "luminance up"},
    {ActionCode::luminanceDown,         "luminance down"},
    {ActionCode::temperatureUp,         "temperature up"},
    {ActionCode::temperatureDown,       "temperature down"},
    {ActionCode::luminance1,            "set luminance"},
    {ActionCode::luminance2,            "set luminance"},
    {ActionCode::color_temperature1,    "set color_temperature"},
    {ActionCode::color_temperature2,    "set color_temperature"}
};


voiceMatch::voiceMatch(const string &ctrlStr) : voiceString(ctrlStr){}

void voiceMatch::parseAndControl(){
     ParsedItem parsedItem;

    //去除包含的字符
    voiceString = voiceMatchUtil::eraseInvalidCharacter(voiceString);
    LOG_INFO << "0> total string: " << voiceString;
   
    //确定动作
    parsedItem.actionCode = voiceMatchUtil::extractAction(voiceString, matchRex2ActionCode, actionCodeCaptureGroup);
    LOG_INFO << "1> after extracting action: " << voiceString;

    //查找确定的设备、分组
    qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
    qlibc::QData groupList = GroupManager::getInstance()->getAllGroupList();
    if(voiceMatchUtil::findDeviceIdOrGroupId(voiceString, deviceList, groupList, deviceTypeMap, parsedItem)){
        LOG_INFO << "2> after extracting device or group: " << voiceString;
    }else{
        LOG_RED << "vocieString matchResult: can not find device or group, match missed.....";
        return;
    }

    //确定参数
    parsedItem.param = voiceMatchUtil::extractParam(voiceString);
    LOG_INFO << "3> after extracting param: " << voiceString;

    //打印控制表项
    printParsedItem(parsedItem);

    //控制设备
    controlByParsedItem(parsedItem);
}

void voiceMatch::printParsedItem(ParsedItem &parsedItem){
    qlibc::QData ids, rooms;
    std::map<string, Json::Value> idsMap = parsedItem.devIdGrpId;
    std::vector<string> roomsMap = parsedItem.roomList;
    for(auto& elem : idsMap){
        ids.append(elem.first);
    }
    for(auto& elem : roomsMap){
        rooms.append(elem);
    }
    
    qlibc::QData data;
    data.setString("action", getActionString(parsedItem.actionCode));
    switch(parsedItem.ctrlType){
        case ControlType::Device :
            data.setString("ctrlType", "Device");
            break;
        case ControlType::Group :
            data.setString("ctrlType", "Group");
            break;
        case ControlType::Type :
            data.setString("ctrlType", "Type");
    }
    data.putData("ids", ids);
    data.putData("param", qlibc::QData(parsedItem.param));
    data.putData("roomName", rooms);
    LOG_PURPLE << "parseItem: " << data.toJsonString(true);
}


string voiceMatch::getActionString(ActionCode code){
    auto pos = actionCode2StringMap.find(code);
    if(pos != actionCode2StringMap.end()){
        return pos->second;
    }else{
        return "NoneAction";
    }
}


Json::Value buildCommandList(ActionCode actionCode, const Json::Value& deviceItem, const ParsedItem& parsedItem){
    CommandItem commandItem;
    if(actionCode == ActionCode::powerOn){   //开
        if(deviceItem["device_type"] == "WINDOW_COVERING_SWITCH"){
            commandItem.command_id = "open";
            commandItem.command_para = "open";
        }else{
            commandItem.command_id = "power";
            commandItem.command_para = "on";
        }
        
    }else if(actionCode == ActionCode::powerOff){    //关
        if(deviceItem["device_type"] == "WINDOW_COVERING_SWITCH"){
            commandItem.command_id = "open";
            commandItem.command_para = "close";
        }else{
            commandItem.command_id = "power";
            commandItem.command_para = "on";
        }


    }else if(actionCode == ActionCode::luminance1 || actionCode == ActionCode::luminance2){   //设置亮度
        commandItem.command_id = "luminance";
        int luminanceValue = static_cast<int>(atoi(parsedItem.param.c_str()) * 2.55);
        commandItem.command_para = luminanceValue;

    }else if(actionCode == ActionCode::color_temperature1 || actionCode == ActionCode::color_temperature2){   //设置色温
        commandItem.command_id = "color_temperature";
        int ctValue = static_cast<int>(atoi(parsedItem.param.c_str()) * 38 + 2700);
        commandItem.command_para = ctValue;

    }else if(actionCode == ActionCode::luminanceUp){   //亮一点
        commandItem.command_id = "luminance_relative";
        commandItem.command_para = 20;

    }else if(actionCode == ActionCode::luminanceDown){   //暗一点
        commandItem.command_id = "luminance_relative";
        commandItem.command_para = -20;

    }else if(actionCode == ActionCode::temperatureUp){   //色温高一点
        commandItem.command_id = "color_temperature_relative";
        commandItem.command_para = 20;

    }else if(actionCode == ActionCode::temperatureDown){  //色温低一点
        commandItem.command_id = "color_temperature_relative";
        commandItem.command_para = -20;
    }
    
    Json::Value command, commandList;
    command["command_id"] = commandItem.command_id;
    command["command_para"] = commandItem.command_para;
    commandList.append(command);
    return commandList;
}


void voiceMatch::controlByParsedItem(ParsedItem& parsedItem){
    qlibc::QData requestData, responseData;
    if(parsedItem.ctrlType == ControlType::Device){
        Json::Value deviceList(Json::arrayValue);
        //控制命令列表
        for(auto& deviceEntry : parsedItem.devIdGrpId){
            Json::Value deviceItem;
            deviceItem["device_id"] = deviceEntry.first;
            deviceItem["command_list"] = buildCommandList(parsedItem.actionCode, deviceEntry.second, parsedItem);
            deviceList.append(deviceItem);
        }

        //将控制命令发送给设备管理站点
        requestData.setString("service_id", "control_device");
        requestData.putData("request", qlibc::QData().setValue("device_list", deviceList));
        LOG_GREEN << "voice controlRequest: " << requestData.toJsonString();
        // httpUtil::sitePostRequest("127.0.0.1", 9007, requestData, responseData);
        // LOG_BLUE << "voice controlResponse: " << responseData.toJsonString();

    }else if(parsedItem.ctrlType == ControlType::Group){
        Json::Value group_list(Json::arrayValue);
        //控制命令列表
        for(auto& groupEntry : parsedItem.devIdGrpId){
            Json::Value groupItem;
            groupItem["group_id"] = groupEntry.first;
            groupItem["command_list"] = buildCommandList(parsedItem.actionCode, groupEntry.second, parsedItem);
            group_list.append(groupItem);
        }

        requestData.setString("service_id", "control_group");
        requestData.putData("request", qlibc::QData().setValue("group_list", group_list));
        LOG_GREEN << "voice controlRequest: " << requestData.toJsonString();
        // httpUtil::sitePostRequest("127.0.0.1", 9007, requestData, responseData);
        // LOG_BLUE << "voice controlResponse: " << responseData.toJsonString();
    }
}



string voiceMatchUtil::eraseInvalidCharacter(const string& str){
    regex sep("[ ]+");
    sregex_token_iterator end;
    sregex_token_iterator p(str.cbegin(), str.cend(), sep, {-1});
    string vaildString{};
    for(; p != end; p++){
        vaildString += p->str();
    }
    return vaildString;
}


std::vector<string> voiceMatchUtil::extractRoom(string& voiceString, const std::vector<string>& roomList){
    std::vector<string> matchedRoomsVec;
    for(auto& room :roomList){
        smatch sm;
        if(regex_search(voiceString, sm, regex(room))){
            matchedRoomsVec.push_back(room);
            voiceString.erase(sm.position(), sm.length());
        }
    }
    return matchedRoomsVec;
}

string voiceMatchUtil::extractDeviceType(string& voiceString, const std::map<string, string> deviceTypeMap){
    for(auto& type : deviceTypeMap){
        if(!voiceString.empty() && !type.first.empty()){
            smatch sm;
            if(regex_search(voiceString, sm, regex(type.first))){
                voiceString.erase(sm.position(), sm.length());
                return type.second;
            }
        }
    }
    return string();
}

string voiceMatchUtil::extractParam(string& voiceString){
    smatch sm;
    if(regex_match(voiceString, sm, regex("[^0-9]*(\\d*)[^0-9]*"))){
        voiceString.erase(sm.position(1), sm.length(1));
        return sm.str(1);
    }else{
        return {};
    }
}

ActionCode voiceMatchUtil::extractAction(string& voiceString, std::map<string, ActionCode> matchRex2ActionCode, std::map<ActionCode, std::vector<int>> actionCodeCaptureGroup){
    for(auto& matchRegx2ActionCodeItem : matchRex2ActionCode){
        smatch sm;
        if(regex_match(voiceString, sm, regex(matchRegx2ActionCodeItem.first))){
            ActionCode code = matchRegx2ActionCodeItem.second;
            auto pos = actionCodeCaptureGroup.find(code);
            if(pos != actionCodeCaptureGroup.end()){
                std::map<string, int> str2DelMap;
                for(auto& captureGroupIndex : pos->second){
                    str2DelMap.insert(std::make_pair(sm.str(captureGroupIndex), sm.length(captureGroupIndex)));
                }
                //删除匹配到的子字符串
                for(auto& elem : str2DelMap){
                    auto pos = voiceString.find(elem.first);
                    if(pos != std::string::npos){
                        voiceString.erase(pos, elem.second);
                    }
                }
                return pos->first;
            }
        }
    }
    return ActionCode::NoneAction;
}

bool voiceMatchUtil::getSpecificDeviceId(string& voiceString, qlibc::QData& deviceList, std::map<string, Json::Value>& matchedDeviceMap) {
    //找到str中包含的设备名称
    std::set<string> deviceNames;
    Json::ArrayIndex size = deviceList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string device_name = item.getString("device_name");
        if(!voiceString.empty() && !device_name.empty()){
            smatch sm;
            if(regex_search(voiceString, sm, regex(device_name))){
                deviceNames.insert(device_name);
                matchedDeviceMap.insert(std::make_pair(item.getString("device_uid"), item.asValue()));
            }
        }
    }

    //从str中剔除设备名字段
    if(matchedDeviceMap.empty()){
        return false;
    }else{
        for(auto& deviceName : deviceNames){
            smatch sm;
            if(regex_search(voiceString, sm, regex(deviceName))){
                voiceString.erase(sm.position(), sm.length());
            }
        }
        return true;
    }
}

bool voiceMatchUtil::getSpecificGroupId(string& voiceString, qlibc::QData& groupList, std::map<string, Json::Value>& matchedGroupMap) {
    std::set<string> groupNames;
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string group_name = item.getString("group_name");
        if(!voiceString.empty() && !group_name.empty()){
            smatch sm;
            if(regex_search(voiceString, sm, regex(group_name))){
                groupNames.insert(group_name);
                matchedGroupMap.insert(std::make_pair( item.getString("group_uid"), item.asValue()));
            }
        }
    }

    //从str中剔除设备名字段
    if(matchedGroupMap.empty()){
        return false;
    }else{
        for(auto& groupName : groupNames){
            smatch sm;
            if(regex_search(voiceString, sm, regex(groupName))){
                voiceString.erase(sm.position(), sm.length());
            }
        }
        return true;
    }
}

bool voiceMatchUtil::getDeviceIdsFromDeviceType(qlibc::QData& deviceList, string& deviceType, std::map<string, Json::Value>& matchedDeviceMap){
    Json::ArrayIndex size = deviceList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string device_type = item.getString("device_type");
        if(deviceType == device_type){
            matchedDeviceMap.insert(std::make_pair(item.getString("device_uid"), item.asValue()));
        }
    }
    return !matchedDeviceMap.empty();
}


bool voiceMatchUtil::getGroupIdsFromRoomName(qlibc::QData& groupList, string& roomName, std::map<string, Json::Value>& matchedGroupMap) {
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string room_name = item.getData("location").getString("room_name");
        if(room_name == roomName){
            matchedGroupMap.insert(std::make_pair(item.getString("group_uid"), item.asValue()));
        }
    }
    return !matchedGroupMap.empty();
}

bool voiceMatchUtil::findDeviceIdOrGroupId(string& voiceString, qlibc::QData& deviceList, qlibc::QData& groupList, 
                        const std::map<string, string> deviceTypeMap, struct ParsedItem& parsedItem){

    std::map<string, Json::Value> deviceIdOrGroupIdMap;
    string deviceType;
    bool isMatch = false;

    if(getSpecificDeviceId(voiceString, deviceList, deviceIdOrGroupIdMap)){     //设备id列表
        parsedItem.ctrlType = ControlType::Device;
        parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
        isMatch = true;

    }else if(getSpecificGroupId(voiceString, groupList, deviceIdOrGroupIdMap)){ //组列表
        parsedItem.ctrlType = ControlType::Group;
        parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
        isMatch = true;

    }else{
        //提取房间 + 类型
        parsedItem.roomList = extractRoom(voiceString, roomList);
        string deviceType = extractDeviceType(voiceString, deviceTypeMap);

        if(!parsedItem.roomList.empty() && !deviceType.empty()){ //房间 + 类型； 组控：控制属于房间的所有的组
            for(string& room : parsedItem.roomList){
                std::map<string, Json::Value> devIdGroupIDMap;
                getGroupIdsFromRoomName(groupList, room, devIdGroupIDMap);
                copy(devIdGroupIDMap.begin(), devIdGroupIDMap.end(), inserter(deviceIdOrGroupIdMap, deviceIdOrGroupIdMap.begin()));
            }
            parsedItem.ctrlType = ControlType::Group;
            parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
            isMatch = true;

        }else if(!deviceType.empty()){  //类型
            if(deviceType == "LIGHT"){  //组控
                parsedItem.ctrlType = ControlType::Group;
                std::set<string> siteNames = SiteRecord::getInstance()->getSiteName();
                for(auto& siteName : siteNames){
                    deviceIdOrGroupIdMap.insert(std::make_pair(string().append("FFFF").append(">").append(siteName), Json::Value()));
                }
                parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
                isMatch = true;

            }else{  //控制指定的单个类型的设备
                std::map<string, Json::Value> matchedDeviceMap;
                getDeviceIdsFromDeviceType(deviceList, deviceType, matchedDeviceMap);
                parsedItem.ctrlType = ControlType::Device;
                parsedItem.devIdGrpId = matchedDeviceMap;
                isMatch = true;
            }
        }
    }
    
    return isMatch;
}




