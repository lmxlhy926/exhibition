
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
    {"灯", "LIGHT"}
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


voiceMatch::voiceMatch(const string &ctrlStr) : voiceString(ctrlStr){}

void voiceMatch::parseAndControl(){
     ParsedItem parsedItem;

    //去除包含的字符
    voiceString = voiceMatchUtil::eraseInvalidCharacter(voiceString);
    LOG_INFO << "0> total string: " << voiceString;
   
    //抽取房间名称，判断是否含有所有字段
    parsedItem.roomNameVec = voiceMatchUtil::extractRoom(voiceString, roomList);
    LOG_INFO << "1> after extracting room: " << voiceString;

    //确定动作
    parsedItem.actionCode = voiceMatchUtil::extractAction(voiceString, matchRex2ActionCode, actionCodeCaptureGroup);
    LOG_INFO << "2> after extracting action: " << voiceString;

    //确定设备、分组

    qlibc::QData deviceList, groupList;


    //查找确定的设备、分组
    if(voiceMatchUtil::findDeviceIdOrGroupId(voiceString, deviceList, groupList, deviceTypeMap, parsedItem)){
        LOG_INFO << "3> after extracting device or group: " << voiceString;
    }else{
        LOG_RED << "vocieString matchResult: can not find device or group, match missed.....";
        return;
    }

    //确定参数
    parsedItem.param = (voiceString);
    LOG_INFO << "4> after extracting param: " << voiceString;

    //打印控制表项
    printParsedItem(parsedItem);

    //控制设备
    controlByParsedItem(parsedItem);

}

void voiceMatch::printParsedItem(ParsedItem &parsedItem){



}


string voiceMatch::code2Action(ActionCode code){
    string action;
    if(code == ActionCode::NoneAction){
        action = "NoneAction";
    }else if(code == ActionCode::powerOn || code == ActionCode::powerOn1){
        action = "powerOn";
    }else if(code == ActionCode::powerOff || code == ActionCode::powerOff1){
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

void voiceMatch::action2Command(ParsedItem& parsedItem, CommandItem& commandItem){
    std::lock_guard<std::mutex> lg(Mutex);

    if(parsedItem.actionCode == ActionCode::powerOn || parsedItem.actionCode == ActionCode::powerOn1){   //开
        commandItem.command_id = "power";
        commandItem.command_para = "on";

    }else if(parsedItem.actionCode == ActionCode::powerOff || parsedItem.actionCode == ActionCode::powerOff1){    //关
        commandItem.command_id = "power";
        commandItem.command_para = "off";

    }else if(parsedItem.actionCode == ActionCode::luminance1 || parsedItem.actionCode == ActionCode::luminance2){   //设置亮度
        commandItem.command_id = "luminance";
        int luminanceValue = static_cast<int>(atoi(parsedItem.param.c_str()) * 2.55);
        commandItem.command_para = luminanceValue;

    }else if(parsedItem.actionCode == ActionCode::color_temperature1 || parsedItem.actionCode == ActionCode::color_temperature2){   //设置色温
        commandItem.command_id = "color_temperature";
        int ctValue = static_cast<int>(atoi(parsedItem.param.c_str()) * 38 + 2700);
        commandItem.command_para = ctValue;

    }else if(parsedItem.actionCode == ActionCode::luminanceUp){   //亮一点
        commandItem.command_id = "luminance_relative";
        commandItem.command_para = 20;

    }else if(parsedItem.actionCode == ActionCode::luminanceDown){   //暗一点
        commandItem.command_id = "luminance_relative";
        commandItem.command_para = -20;

    }else if(parsedItem.actionCode == ActionCode::temperatureUp){   //色温高一点
        commandItem.command_id = "color_temperature_relative";
        commandItem.command_para = 20;

    }else if(parsedItem.actionCode == ActionCode::temperatureDown){  //色温低一点
        commandItem.command_id = "color_temperature_relative";
        commandItem.command_para = -20;
    }
}


void voiceMatch::controlByParsedItem(ParsedItem& parsedItem){
    qlibc::QData requestData, responseData;
    if(parsedItem.ctrlType == ControlType::Device){
        Json::Value command, commandList(Json::arrayValue), deviceItem, deviceList(Json::arrayValue);
        //单个控制命令项
        CommandItem commandItem;
        action2Command(parsedItem, commandItem);
        command["command_id"] = commandItem.command_id;
        command["command_para"] = commandItem.command_para;
        commandList.append(command);

        //控制命令列表
        for(auto& deviceId : parsedItem.devIdGrpId){
            deviceItem["device_id"] = deviceId.first;
            deviceItem["command_list"] = commandList;
            deviceList.append(deviceItem);
        }

        //将控制命令发送给设备管理站点
        requestData.setString("service_id", "control_device");
        requestData.putData("request", qlibc::QData().setValue("device_list", deviceList));
        LOG_GREEN << "voice controlRequest: " << requestData.toJsonString();
        httpUtil::sitePostRequest("127.0.0.1", 9007, requestData, responseData);
        LOG_BLUE << "voice controlResponse: " << responseData.toJsonString();

    }else if(parsedItem.ctrlType == ControlType::Group){
        Json::Value command, commandList(Json::arrayValue), groupItem, group_list(Json::arrayValue);
        //控制命令项
        CommandItem commandItem;
        action2Command(parsedItem, commandItem);
        command["command_id"] = commandItem.command_id;
        command["command_para"] = commandItem.command_para;
        commandList.append(command);

        //控制命令列表
        for(auto& groupId : parsedItem.devIdGrpId){
            groupItem["group_id"] = groupId.first;
            groupItem["command_list"] = commandList;
            group_list.append(groupItem);
        }

        requestData.setString("service_id", "control_group");
        requestData.putData("request", qlibc::QData().setValue("group_list", group_list));
        LOG_GREEN << "voice controlRequest: " << requestData.toJsonString();
        httpUtil::sitePostRequest("127.0.0.1", 9007, requestData, responseData);
        LOG_BLUE << "voice controlResponse: " << responseData.toJsonString();
    }
}


namespace voiceMatchUtil{
    string eraseInvalidCharacter(const string& str){
        regex sep("[ ]+");
        sregex_token_iterator end;
        sregex_token_iterator p(str.cbegin(), str.cend(), sep, {-1});
        string vaildString{};
        for(; p != end; p++){
            vaildString += p->str();
        }
        return vaildString;
    }


    std::vector<string> extractRoom(string& voiceString, const std::vector<string>& roomList){
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

    string extractDeviceType(string& voiceString, const std::map<string, string> deviceTypeMap){
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

    string extractParam(string& voiceString){
        smatch sm;
        if(regex_match(voiceString, sm, regex("[^0-9]*(\\d*)[^0-9]*"))){
            voiceString.erase(sm.position(1), sm.length(1));
            return sm.str(1);
        }else{
            return {};
        }
    }

    ActionCode extractAction(string& voiceString, std::map<string, ActionCode> matchRex2ActionCode, std::map<ActionCode, std::vector<int>> actionCodeCaptureGroup){
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

    bool getSpecificDeviceId(string& voiceString, qlibc::QData& deviceList, std::map<string, Json::Value>& matchedDeviceMap) {
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

    bool getSpecificGroupId(string& voiceString, qlibc::QData& groupList, std::map<string, Json::Value>& matchedGroupMap) {
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

    bool getDeviceIdsFromDeviceType(qlibc::QData& deviceList, string& deviceType, std::map<string, Json::Value>& matchedDeviceMap){
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


    bool getGroupIdsFromRoomName(qlibc::QData& groupList, string& roomName, std::map<string, Json::Value>& matchedGroupMap) {
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

    bool findDeviceIdOrGroupId(string& voiceString, qlibc::QData& deviceList, qlibc::QData& groupList, 
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
            string deviceType = extractDeviceType(voiceString, deviceTypeMap);
            if(!parsedItem.roomNameVec.empty() && !deviceType.empty()){ //房间 + 类型
                for(string& room : parsedItem.roomNameVec){
                    std::map<string, Json::Value> devIdGroupIDMap;
                    getGroupIdsFromRoomName(groupList, room, devIdGroupIDMap);
                    copy(devIdGroupIDMap.begin(), devIdGroupIDMap.end(), inserter(deviceIdOrGroupIdMap, deviceIdOrGroupIdMap.begin()));
                }
                parsedItem.ctrlType = ControlType::Group;
                parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
                isMatch = true;

            }else if(!deviceType.empty()){  //类型
                if(deviceType == "LIGHT"){
                    parsedItem.ctrlType = ControlType::Group;
                    std::set<string> siteNames = SiteRecord::getInstance()->getSiteName();
                    for(auto& siteName : siteNames){
                        deviceIdOrGroupIdMap.insert(std::make_pair(string().append("FFFF").append(">").append(siteName), Json::Value()));
                    }
                    parsedItem.devIdGrpId = deviceIdOrGroupIdMap;
                    isMatch = true;

                }else{
                    //找到所有的窗帘
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
}





