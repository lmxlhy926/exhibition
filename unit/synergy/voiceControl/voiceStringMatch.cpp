//
// Created by 78472 on 2022/12/11.
//

#include "voiceStringMatch.h"
#include <regex>

//开关灯
regex voice_expr_power_on("(.*)(灯)(.*)(开)(.*)");
regex voice_expr_power_on_1("(.*)(开)(.*)(灯)(.*)");
regex voice_expr_power_off("(.*)(灯)(.*)(关)(.*)");
regex voice_expr_power_off_1("(.*)(关)(.*)(灯)(.*)");


void voiceStringMatchControl::controlDevice() {
    smatch sm;
    //开关指令
    if(regex_match(controlString, sm, voice_expr_power_on) ||
       regex_match(controlString, sm, voice_expr_power_on_1))
    {
        string deviceIdOrGroupId;
        bool isMatch = false;
        MatchItem matchItem;
        matchItem.command_id = "power";
        matchItem.command_para = "on";

        for(int i = 1; i < sm.size(); ++i){
            if(getSpecificDeviceId(sm[i].str(), deviceIdOrGroupId)){
                matchItem.type = VoiceMatchType::device;
                matchItem.id = deviceIdOrGroupId;
                isMatch = true;
                break;

            }else if(getSpecificGroupId(sm[i].str(), deviceIdOrGroupId)){
                matchItem.type = VoiceMatchType::group;
                matchItem.id = deviceIdOrGroupId;
                isMatch = true;
                break;

            }else if(getGroupIdFromRoomName(sm[i].str(), deviceIdOrGroupId)){
                matchItem.type = VoiceMatchType::group;
                matchItem.id = deviceIdOrGroupId;
                isMatch = true;
                break;

            }else if(getFullGroupId(sm[i].str(), deviceIdOrGroupId)){
                matchItem.type = VoiceMatchType::group;
                matchItem.id = "FFFF";
                isMatch = true;
                break;
            }
        }

        if(isMatch){
            controlDeviceOrGroup(matchItem);
        }
    }

}
