//
// Created by 78472 on 2022/12/11.
//

#ifndef EXHIBITION_VOICESTRINGMATCH_H
#define EXHIBITION_VOICESTRINGMATCH_H

#include <string>
#include <regex>
#include "qlibc/QData.h"
using namespace std;

//动作码：指示相应的控制命令
enum ActionCode{
    NoneAction = 0,
    powerOn,
    powerOff,
    setColor
};

enum DeviceType{
    NoneDeviceType = 0,
    LIGHT
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
    string roomName;
    ActionCode actionCode = NoneAction;
    ControlType ctrlType = NoneType;
    string devIdGrpId;
    enum DeviceType deviceType;
    string param;
    string sourceSite;
};

//单个控制项
struct CommandItem{
    string command_id;
    Json::Value command_para;
};

class voiceStringMatchControl {
private:
    string controlParseString;
    string voiceString;
    std::vector<string> roomList;                      //房间列表
    std::map<string, DeviceType> deviceTypeMap;       //设备类型列表
    std::map<string, ActionCode> matchRex2ActionCode;               //正则表达式--> 控制码
    std::map<ActionCode, std::vector<int>> actionCodeCaptureGroup;  //控制码 --- 捕获分组

public:
    explicit voiceStringMatchControl(string& ctrlStr);

    //匹配字符串，控制设备
    void parseAndControl();

private:
    //控制码---> 控制动作
    string code2Action(ActionCode code);

    //类型码--->类型说明
    string deviceType2String(DeviceType deviceType);

    //打印解析结构
    void printParsedItem(struct ParsedItem& item);

    //控制动作转换为控制命令
    void action2Command(ParsedItem& parsedItem, CommandItem& commandItem);

    //获取设备名称相匹配的设备ID
    bool getSpecificDeviceId(qlibc::QData& deviceList, string& str, string& deviceId, string& sourceSite);

    //获取组名称相匹配的组ID
    bool getSpecificGroupId(qlibc::QData& groupList, string& str, string& groupId, string& sourceSite);

    //获取控制的类型
    bool getDeviceType(string& str, DeviceType& deviceType);

    //获取房间的默认组ID
    bool getGroupIdFromRoomNameAndType(qlibc::QData& groupList, string& roomName, string deviceType, string& groupId, string& sourceSite);

    //找到设备id或者组id
    bool findDeviceIdOrGroupId(string& str, ParsedItem& parsedItem);

    //依据解析项进行控制
    void controlByParsedItem(ParsedItem& parsedItem);
};


#endif //EXHIBITION_VOICESTRINGMATCH_H
