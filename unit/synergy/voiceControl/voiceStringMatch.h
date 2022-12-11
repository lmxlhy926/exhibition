//
// Created by 78472 on 2022/12/11.
//

#ifndef EXHIBITION_VOICESTRINGMATCH_H
#define EXHIBITION_VOICESTRINGMATCH_H

#include <string>
using namespace std;

enum VoiceMatchType{
    device,
    group
};

struct MatchItem{
    enum VoiceMatchType type;
    string id;
    string command_id;
    string command_para;
};

class voiceStringMatchControl {
private:
    string controlString;
public:
    explicit voiceStringMatchControl(string& ctrlStr) : controlString(ctrlStr){}

    //匹配字符串，控制设备
    void controlDevice();

private:
    //

    //获取设备名称相匹配的设备ID
    bool getSpecificDeviceId(string parseStr, string& deviceId);

    //获取组名称相匹配的组ID
    bool getSpecificGroupId(string parseStr, string& groupId);

    //获取房间的默认组ID
    bool getGroupIdFromRoomName(string parseStr, string& groupId);

    //所有设备
    bool getFullGroupId(string parseStr, string& groupId);

    //控制对应的设备或者组
    void controlDeviceOrGroup(MatchItem& matchItem);
};


#endif //EXHIBITION_VOICESTRINGMATCH_H
