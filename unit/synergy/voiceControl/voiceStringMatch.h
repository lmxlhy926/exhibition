//
// Created by 78472 on 2022/12/11.
//

#ifndef EXHIBITION_VOICESTRINGMATCH_H
#define EXHIBITION_VOICESTRINGMATCH_H

#include <string>
#include <regex>
#include "qlibc/QData.h"


using namespace std;

enum VoiceMatchType{
    Device,
    Group,
    WrongType
};

struct MatchItem{
    enum VoiceMatchType type = VoiceMatchType::WrongType;
    string id;
    string command_id;
    string command_para;
    string sourceSite;
};

class voiceStringMatchControl {
private:
    string controlString;
public:
    explicit voiceStringMatchControl(string& ctrlStr) : controlString(ctrlStr){}

    //匹配字符串，控制设备
    void parseAndControl();

private:
    //获取设备名称相匹配的设备ID
    bool getSpecificDeviceId(qlibc::QData& deviceList, string parseStr, string& deviceId, string& sourceSite);

    //获取组名称相匹配的组ID
    bool getSpecificGroupId(qlibc::QData& groupList, string parseStr, string& groupId, string& sourceSite);

    //获取房间的默认组ID
    bool getGroupIdFromRoomName(qlibc::QData& groupList, string parseStr, string& groupId, string& sourceSite);

    //所有设备
    bool getFullGroupId(string parseStr, string& groupId, string& sourceSite);

    //打印控制结构
    void printMatchItem(MatchItem& matchItem);

    //解析字符串--> MatchItem
    bool parse2MatchItem(smatch& sm, MatchItem& matchItem);

    //依据MatchItem，控制设备
    void controlDeviceOrGroupByMatchItem(MatchItem& matchItem);

};


#endif //EXHIBITION_VOICESTRINGMATCH_H
