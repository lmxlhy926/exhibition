
#include <set>
#include <map>
#include "voiceMatch.h"
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "common/httpUtil.h"
#include "../param.h"
#include "../sourceManage/deviceManager.h"
#include "../sourceManage/groupManager.h"

static std::set<string> roomList{};

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
    //获取设备列表、组列表、刷新房间列表
    qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
    qlibc::QData groupList = GroupManager::getInstance()->getAllGroupList();
    voiceMatchUtil::refreshRoomList(deviceList, groupList);

    ParsedItem parsedItem;

    //去除包含的字符
    voiceString = voiceMatchUtil::eraseInvalidCharacter(voiceString);
    LOG_INFO << "> total string: " << voiceString;

    //如果有房间，提取动作时，房间信息不参与提取
    std::map<string, string> restultMap = voiceMatchUtil::preExtractRoom(voiceString, roomList);
    if(!restultMap.empty()){
        string prefix = restultMap.find("prefix")->second;
        string room = restultMap.find("room")->second;
        string suffix = restultMap.find("suffix")->second;

        LOG_INFO << "prefix: " << prefix;
        LOG_INFO << "room: " << room;
        LOG_INFO << "suffix: " << suffix;

        ActionCode actionCodePrefix = voiceMatchUtil::extractAction(prefix, matchRex2ActionCode, actionCodeCaptureGroup);
        ActionCode actionCodeSuffix = voiceMatchUtil::extractAction(suffix, matchRex2ActionCode, actionCodeCaptureGroup);
        if(actionCodePrefix == ActionCode::NoneAction && actionCodeSuffix == ActionCode::NoneAction){
            LOG_RED << "vocieString matchResult: can not find action, match missed.....";
            return;
        }

        if(actionCodePrefix != ActionCode::NoneAction){
            parsedItem.actionCode = actionCodePrefix;
            voiceString = prefix + room + restultMap.find("suffix")->second;

        }else if(actionCodeSuffix != ActionCode::NoneAction){
            parsedItem.actionCode = actionCodeSuffix;
            voiceString = restultMap.find("prefix")->second + room + suffix;
        }

    }else{
        //确定动作
        parsedItem.actionCode = voiceMatchUtil::extractAction(voiceString, matchRex2ActionCode, actionCodeCaptureGroup);
        if(parsedItem.actionCode == ActionCode::NoneAction){
            LOG_RED << "vocieString matchResult: can not find action, match missed.....";
            return;
        }
    }
    LOG_INFO << "> after extracting action: " << voiceString;
    
    if(voiceMatchUtil::findDeviceIdOrGroupId(voiceString, deviceList, groupList, deviceTypeMap, parsedItem)){
        LOG_INFO << "> after extracting deviceName groupName roomName: " << voiceString;
    }else{
        LOG_RED << "vocieString matchResult: can not find device or group or matchedTypeResult, match missed.....";
        return;
    }

    //确定参数
    parsedItem.param = voiceMatchUtil::extractParam(voiceString);
    LOG_INFO << "> after extracting param: " << voiceString;

    //打印控制表项
    printParsedItem(parsedItem);

    //控制设备
    controlByParsedItem(parsedItem);
}

void voiceMatch::printParsedItem(ParsedItem &parsedItem){
    qlibc::QData ids, rooms;
    std::map<string, Json::Value> idsMap = parsedItem.devIdGrpId;
    std::set<string> roomsSet = parsedItem.roomList;
    for(auto& elem : idsMap){
        ids.append(elem.first);
    }
    for(auto& elem : roomsSet){
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
    data.setString("hasAll", parsedItem.containsAll ? "true" : "false");
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
            commandItem.command_para = "off";
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
        httpUtil::sitePostRequest("127.0.0.1", 9007, requestData, responseData);
        LOG_BLUE << "voice controlResponse: " << responseData.toJsonString();

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
        httpUtil::sitePostRequest("127.0.0.1", 9007, requestData, responseData);
        LOG_BLUE << "voice controlResponse: " << responseData.toJsonString();
    }
}


std::recursive_mutex voiceMatchUtil::rMutex;
void voiceMatchUtil::refreshRoomList(qlibc::QData& deviceList, qlibc::QData& groupList){
    std::lock_guard<std::recursive_mutex> lg(rMutex);
    Json::ArrayIndex deviceListSize = deviceList.size();
    for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string roomName = item.getData("location").getString("room_name");
        roomList.insert(roomName);
    }

    Json::ArrayIndex groupListSize = groupList.size();
    for(Json::ArrayIndex i = 0; i < groupListSize; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string roomName = item.getData("location").getString("room_name");
        roomList.insert(roomName);
    }
}


map<string, string> voiceMatchUtil::preExtractRoom(const string& voiceString, const std::set<string>& roomList){
    //匹配第一个符号要求的房间名
    std::map<string, std::map<string, string>> matchedMap;
    for(auto& room :roomList){
        smatch sm;
        string regexString = "(.*)" + room +  "(.*)";              
        if(regex_match(voiceString, sm, regex(regexString))){
            std::map<string, string> resultMap;
            resultMap.insert(std::make_pair("prefix", sm.str(1)));
            resultMap.insert(std::make_pair("room", room));
            resultMap.insert(std::make_pair("suffix", sm.str(2)));
            matchedMap.insert(std::make_pair(room, resultMap));   
        }   
    }

    if(matchedMap.empty()){
        return {};
    }

    //最长名称匹配
    string maxLengthName;
    for(auto& matchedMapElem : matchedMap){
        if(maxLengthName.size() < matchedMapElem.first.size()){
            maxLengthName = matchedMapElem.first;
        } 
    }

    return matchedMap[maxLengthName];
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


bool voiceMatchUtil::containsAll(string& voiceString, struct ParsedItem& parsedItem){
    auto allPos = voiceString.find("所有");
    if(allPos != std::string::npos){
        voiceString.erase(allPos, string("所有").length());
        parsedItem.containsAll = true;
        return true;
    }
    return false;
}


std::set<string> voiceMatchUtil::extractRoom(string& voiceString, const std::set<string>& roomList){
    //匹配第一个符号要求的房间名
    std::set<string> matchedRoomsVec;
    for(auto& room :roomList){
        smatch sm;              
        if(regex_search(voiceString, sm, regex(room))){
            if(matchedRoomsVec.empty()){
                 matchedRoomsVec.insert(room);
            }else{
                if(matchedRoomsVec.begin()->size() < room.size()){
                    matchedRoomsVec.erase(matchedRoomsVec.begin());
                    matchedRoomsVec.insert(room);
                }
            }
        }
    }

    if(!matchedRoomsVec.empty()){
        string roomName = *matchedRoomsVec.begin();
        smatch sm;
        if(regex_search(voiceString, sm, regex(roomName))){
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

bool voiceMatchUtil::getSpecificDeviceId(string& voiceString, qlibc::QData& deviceList, struct ParsedItem& parsedItem,
                                         std::map<string, Json::Value>& matchedDeviceMap) {
    //找到str中包含的设备名称
    std::set<string> deviceNames;
    Json::ArrayIndex size = deviceList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string device_name = item.getString("device_name");
        
        if(!voiceString.empty() && !device_name.empty()){
            smatch sm;
            if(regex_search(voiceString, sm, regex(device_name))){
                /**
                 * 名字长度最大匹配策略
                 *      * 匹配最长的名字
                 *      * 相同长度的名字匹配为第一个匹配到的名字
                */
                if(deviceNames.empty()){
                    deviceNames.insert(device_name);
                    matchedDeviceMap.insert(std::make_pair(item.getString("device_uid"), item.asValue()));
                }else{
                    if(deviceNames.begin()->size() < device_name.size()){
                        deviceNames.erase(deviceNames.begin());
                        deviceNames.insert(device_name);
                        matchedDeviceMap.insert(std::make_pair(item.getString("device_uid"), item.asValue()));

                    }else if(*deviceNames.begin() == device_name){
                         matchedDeviceMap.insert(std::make_pair(item.getString("device_uid"), item.asValue()));
                    }
                }
            }
        }
    }

    //匹配失败
    if(deviceNames.empty()){
        return false;
    }

    //删除名称不符合的设备
    string matchedDeviceName = *deviceNames.begin();
    for(auto pos = matchedDeviceMap.begin(); pos != matchedDeviceMap.end(); ){
        if(pos->second["device_name"].asString() != matchedDeviceName){
            pos = matchedDeviceMap.erase(pos);
        }else{
            ++pos;
        }
    }

    //从str中剔除设备名字段
    if(matchedDeviceMap.empty()){
        return false;
    }else{
        //去除设备名称字段
        smatch sm;
        if(regex_search(voiceString, sm, regex(matchedDeviceName))){
            voiceString.erase(sm.position(), sm.length());
        }

        //匹配房间，如果匹配到房间，则过滤不属于房间的设备
        parsedItem.roomList = voiceMatchUtil::extractRoom(voiceString, roomList);
        if(!parsedItem.roomList.empty()){
            string roomName = *parsedItem.roomList.begin();
            for(auto pos = matchedDeviceMap.begin(); pos != matchedDeviceMap.end(); ){
                if(pos->second["location"]["room_name"].asString() != roomName){
                    pos = matchedDeviceMap.erase(pos);
                }else{
                    ++pos;
                }
            }
        }

        return true;
    }
}

bool voiceMatchUtil::getSpecificGroupId(string& voiceString, qlibc::QData& groupList, struct ParsedItem& parsedItem,
                                        std::map<string, Json::Value>& matchedGroupMap) {
    std::set<string> groupNames;
    Json::ArrayIndex size = groupList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = groupList.getArrayElement(i);
        string group_name = item.getString("group_name");
       
        if(!voiceString.empty() && !group_name.empty()){
            smatch sm;
            if(regex_search(voiceString, sm, regex(group_name))){

                /**
                 * 名字长度最大匹配策略
                 *      * 匹配最长的名字
                 *      * 相同长度的名字匹配为第一个匹配到的名字
                */
                if(groupNames.empty()){
                    groupNames.insert(group_name);
                    matchedGroupMap.insert(std::make_pair(item.getString("group_uid"), item.asValue()));
                }else{
                    if(groupNames.begin()->size() < group_name.size()){
                        groupNames.erase(groupNames.begin());
                        groupNames.insert(group_name);
                        matchedGroupMap.insert(std::make_pair(item.getString("group_uid"), item.asValue()));

                    }else if(*groupNames.begin() == group_name){
                         matchedGroupMap.insert(std::make_pair(item.getString("group_uid"), item.asValue()));
                    }
                }
            }
        }
    }

    //匹配失败
    if(groupNames.empty()){
        return false;
    }

    //删除名称不符合的设备
    string matchedGroupName = *groupNames.begin();
    for(auto pos = matchedGroupMap.begin(); pos != matchedGroupMap.end(); ){
        if(pos->second["group_name"].asString() != matchedGroupName){
            pos = matchedGroupMap.erase(pos);
        }else{
            ++pos;
        }
    }

    //从str中剔除设备名字段
    if(matchedGroupMap.empty()){
        return false;
    }else{
        //去除组名称字段
        smatch sm;
        if(regex_search(voiceString, sm, regex(matchedGroupName))){
            voiceString.erase(sm.position(), sm.length());
        }

        //匹配房间，如果匹配到房间，则过滤不属于房间的组
        parsedItem.roomList = voiceMatchUtil::extractRoom(voiceString, roomList);
        if(!parsedItem.roomList.empty()){
            string roomName = *parsedItem.roomList.begin();
            for(auto pos = matchedGroupMap.begin(); pos != matchedGroupMap.end(); ){
                if(pos->second["location"]["room_name"].asString() != roomName){
                    pos = matchedGroupMap.erase(pos);
                }else{
                    ++pos;
                }
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

bool voiceMatchUtil::getDeviceIdsFromRoomNameAndDeviceType(qlibc::QData& deviceList, const string& roomName, const string& deviceType,
                                                           std::map<string, Json::Value>& matchedDeviceMap){
    Json::ArrayIndex size = deviceList.size();
    for(Json::ArrayIndex i = 0; i < size; ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        string device_type = item.getString("device_type");
        string room_name = item.getData("location").getString("room_name");
        if(room_name == roomName && device_type == deviceType){
            matchedDeviceMap.insert(std::make_pair(item.getString("device_uid"), item.asValue()));
        }
    }
    return !matchedDeviceMap.empty();
}

bool voiceMatchUtil::getGroupIdsFromRoomName(qlibc::QData& groupList, const string& roomName, std::map<string, Json::Value>& matchedGroupMap) {
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

    if(getSpecificDeviceId(voiceString, deviceList, parsedItem, deviceIdOrGroupIdMap)){     //设备id列表
        parsedItem.ctrlType = ControlType::Device;
        parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
        isMatch = true;

    }else if(getSpecificGroupId(voiceString, groupList, parsedItem, deviceIdOrGroupIdMap)){ //组列表
        parsedItem.ctrlType = ControlType::Group;
        parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
        isMatch = true;

    }else{
        //提取房间 + 类型
        parsedItem.roomList = extractRoom(voiceString, roomList);
        bool hasAll = containsAll(voiceString, parsedItem);
        string deviceType = extractDeviceType(voiceString, deviceTypeMap);

        if(!parsedItem.roomList.empty() && !deviceType.empty()){ //房间 + 类型； 
            if(deviceType == "LIGHT"){  //组控：控制属于房间的所有的组
                for(const string& room : parsedItem.roomList){
                    std::map<string, Json::Value> devIdGroupIDMap;
                    getGroupIdsFromRoomName(groupList, room, devIdGroupIDMap);
                    copy(devIdGroupIDMap.begin(), devIdGroupIDMap.end(), inserter(deviceIdOrGroupIdMap, deviceIdOrGroupIdMap.begin()));
                }
                parsedItem.ctrlType = ControlType::Group;
                parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
                isMatch = true;

            }else{  //控制房间的所有的单个设备
                for(const string& room : parsedItem.roomList){
                    std::map<string, Json::Value> devIdGroupIDMap;
                    getDeviceIdsFromRoomNameAndDeviceType(deviceList, room, deviceType, devIdGroupIDMap);
                    copy(devIdGroupIDMap.begin(), devIdGroupIDMap.end(), inserter(deviceIdOrGroupIdMap, deviceIdOrGroupIdMap.begin()));
                }
                parsedItem.ctrlType = ControlType::Device;
                parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
                isMatch = true;
            }

        }else if(parsedItem.roomList.empty() && !deviceType.empty() && hasAll){  //类型 + '所有'
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




