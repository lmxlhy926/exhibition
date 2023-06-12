//
// Created by 78472 on 2022/12/11.
//

#ifndef EXHIBITION_VOICEMATCH_H
#define EXHIBITION_VOICEMATCH_H

#include <string>
#include <regex>
#include "qlibc/QData.h"
using namespace std;

//动作码：指示相应的控制命令
enum ActionCode{
    NoneAction = 0,
    powerOn,
    powerOff,
    luminance1,
    luminance2,
    color_temperature1,
    color_temperature2,
    luminanceUp,
    luminanceDown,
    temperatureUp,
    temperatureDown
};

//控制类型：设备、组、类型
enum ControlType{
    NoneType = 0,
    Device,
    Group,
    Type
};

//解析项：房间名、动作码、<设备号、组id、类型>、动作参数
struct ParsedItem{
    ActionCode actionCode = ActionCode::NoneAction;      //控制动作码
    ControlType ctrlType = ControlType::NoneType;        //控制类型，单个设备or分组
    std::map<string, Json::Value> devIdGrpId;            //匹配的设备ids,组ids；
    string param;                                        //控制参数
    std::vector<string> roomList;                        //所在房间
};


//单个控制命令项
struct CommandItem{
    string command_id;
    Json::Value command_para;
};


class voiceMatch{
private:
    string voiceString;
    static int tempLuminance;
    static int tempTemperature;
    const int defaultLuminance = 60;
    const int deltaLuminance = 20;
    const int defaultTemperature = 60;
    const int deltaTemperature = 20;
public:
    explicit voiceMatch(const string& ctrlStr);

    void parseAndControl();

    //获取动作码描述字符串
    string getActionString(ActionCode code);

    //打印解析结果
    void printParsedItem(ParsedItem& parsedItem);

    //解析并控制
    void controlByParsedItem(ParsedItem& parsedItem);
};


class voiceMatchUtil{
public:
    //移除控制字符串中的无效字符
    static string eraseInvalidCharacter(const string& str);

    //提取控制字符串中的房间
    static std::vector<string> extractRoom(string& voiceString, const std::vector<string>& roomList);

    //提取控制字符串中的控制设备类型
    static string extractDeviceType(string& voiceString, const std::map<string, string> deviceTypeMap);

    //提取控制字符串中的控制参数
    static string extractParam(string& voiceString);

    //提取控制字符串中的控制动作码
    static ActionCode extractAction(string& voiceString, std::map<string, ActionCode> matchRex2ActionCode, std::map<ActionCode, std::vector<int>> actionCodeCaptureGroup);

    //提取匹配的设备ID
    static bool getSpecificDeviceId(string& voiceString, qlibc::QData& deviceList, std::map<string, Json::Value>& matchedDeviceMap);

    //提取匹配的组ID
    static bool getSpecificGroupId(string& voiceString, qlibc::QData& groupList, std::map<string, Json::Value>& matchedGroupMap);

    //从设备列表中获取类型匹配的设备
    static bool getDeviceIdsFromDeviceType(qlibc::QData& deviceList, string& deviceType, std::map<string, Json::Value>& matchedDeviceMap);

    //从组列表中提取房间名匹配的组
    static bool getGroupIdsFromRoomName(qlibc::QData& groupList, string& roomName, std::map<string, Json::Value>& matchedGroupMap);

    // 提取匹配到的设备或组
    static bool findDeviceIdOrGroupId(string& voiceString, qlibc::QData& deviceList, qlibc::QData& groupList, 
                               const std::map<string, string> deviceTypeMap, struct ParsedItem& parsedItem);
};

#endif //EXHIBITION_VOICEMATCH_H
