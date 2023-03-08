//
// Created by 78472 on 2022/12/11.
//

#ifndef EXHIBITION_VOICESTRINGMATCH_H
#define EXHIBITION_VOICESTRINGMATCH_H

#include <string>
#include <regex>
#include "qlibc/QData.h"
#include <mutex>
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
    string roomName;
    bool hasAll{false};
    ActionCode actionCode = NoneAction;     //控制动作码
    ControlType ctrlType = NoneType;        //控制类型，单个设备or分组
    std::vector<string> devIdGrpId;
    string deviceType;
    string param;
};

//单个控制命令项
struct CommandItem{
    string command_id;
    Json::Value command_para;
};

class voiceStringMatchControl {
private:
    string controlParseString;
    string voiceString;
    std::vector<string> roomList;                      //房间列表
    std::map<string, string> deviceTypeMap;             //设备类型列表
    std::map<string, ActionCode> matchRex2ActionCode;               //正则表达式--> 控制码
    std::map<ActionCode, std::vector<int>> actionCodeCaptureGroup;  //控制码 --- 捕获分组

    static string lastModifyedDeviceOrGroup;
    static int tempLuminance;
    static int tempTemperature;
    const int defaultLuminance = 60;
    const int deltaLuminance = 20;
    const int defaultTemperature = 60;
    const int deltaTemperature = 20;
    std::mutex Mutex;   //保护静态变量

public:
    explicit voiceStringMatchControl(string& ctrlStr);

    //匹配字符串，控制设备
    void parseAndControl();

private:
    //动作控制码---> 控制动作字符串
    string code2Action(ActionCode code);

    //打印解析结构
    void printParsedItem(struct ParsedItem& item);

    //从解析项中提取控制命令项
    void action2Command(ParsedItem& parsedItem, CommandItem& commandItem);

    //获取与设备名称相匹配的设备ID
    bool getSpecificDeviceId(qlibc::QData& deviceList, string& str, std::vector<string>& deviceVec);

    //获取与组名称相匹配的组ID
    bool getSpecificGroupId(qlibc::QData& groupList, string& str, std::vector<string>& groupVec);

    //获取指定房间对应的组
    bool getGroupIdFromRoomName(qlibc::QData& groupList, string& roomName, std::vector<string>& groupVec);

    //获取控制的类型
    bool getDeviceType(string& str, string& deviceType);

    //找到设备id或者组id，填充入解析项
    bool findDeviceIdOrGroupId(string& str, ParsedItem& parsedItem);

    //依据解析项进行控制
    void controlByParsedItem(ParsedItem& parsedItem);
};


#endif //EXHIBITION_VOICESTRINGMATCH_H
