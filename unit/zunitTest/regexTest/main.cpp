//
// Created by 78472 on 2022/12/11.
//

#include <regex>
#include <iostream>
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <vector>
#include "qlibc/QData.h"
using namespace std;

//开关灯
regex voice_expr_power_on("(.*)(灯)(.*)(开)(.*)");
regex voice_expr_power_on_1("(.*)(开)(.*)(灯)(.*)");
regex voice_expr_power_off("(.*)(灯)(.*)(关)(.*)");
regex voice_expr_power_off_1("(.*)(关)(.*)(灯)(.*)");


static string deleteWhiteSpace(string str){
    string retStr;
    regex sep(" ");
    sregex_token_iterator p(str.cbegin(), str.cend(), sep, -1);
    sregex_token_iterator e;
    for(; p != e; ++p){
        retStr.append(*p);
    }
    return retStr;
}

static int getArbitrary(int num){
    time_t t;
    srand(time(&t));
    return rand() % num;
}

string getBinaryString(){
    string netkeyBase = "11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E";
    int arbitrary = getArbitrary(10000);
    stringstream ss;
    ss << std::setw(4) << std::setfill('0') << std::hex << std::uppercase << arbitrary;
    string network_key = netkeyBase + ss.str();
    string binaryString = "E9 FF 09" + network_key + "00 00" + "00" + "11 22 33 44" + "00 01";
    return deleteWhiteSpace(binaryString);
}



/*
 * 动作码：动作码和相应的控制命令相对应
 */
enum ActionCode{
    NoneAction = 0,
    open,
    close,
    setColor
};

/*
 * 控制类型：
 *      设备
 *      组
 *      类型
 */
enum ControlType{
    NoneType = 0,
    Device,
    Group,
    Type
};

/*
 * 参数类型
 */
enum ActionParamType{
    None = 0,
    IntType,
    StringType
};

/*
 * 解析项:
 *      房间名，
 *      动作码，
 *      设备号，组id，类型
 *      动作参数
 */
struct ParsedItem{
    string roomName;
    ActionCode actionCode = NoneAction;
    ControlType ctrlType = NoneType;
    string devGrpType;
    string param;
};


/*
 * 房间列表
 */
std::vector<string> roomVec{
    "客厅",
    "厨房",
    "卧室"
};

/*
 * 动作码--->捕获分组
 */
std::map<ActionCode, std::vector<int>> code2CaptureGrpMap{
        {ActionCode::open, {1}},
        {ActionCode::close, {1}},
        {ActionCode::setColor, {1, 4}}
};


/*
 * 匹配字符串--->动作码
 */
std::map<string, ActionCode> action2CodeMap{
        {".*(打开).*", ActionCode::open},
        {".*(关闭).*", ActionCode::close},
        {".*((调|变|换|设)(到|成|为|置)).*(色温).*", ActionCode::setColor},
        {".*(色温).*((调|变|换|设)(到|成|为|置)).*", ActionCode::setColor},
        {".*((调|变|换|设)(到|成|为|置)).*(亮度).*", ActionCode::setColor},
        {".*(亮度).*((调|变|换|设)(到|成|为|置)).*", ActionCode::setColor}
};


string code2Action(ActionCode code){
    string action;
    if(code == ActionCode::NoneAction){
        action = "NoneAction";
    }else if(code == ActionCode::open){
        action = "open";
    }else if(code == ActionCode::close){
        action = "close";
    }else if (code == ActionCode::setColor){
        action = "setColor";
    }
    return action;
}

void printParsedItem(struct ParsedItem& item){
    stringstream  ss;
    ss  << "roomName: " << item.roomName    << ", "
        << "action: "   << code2Action(item.actionCode)  << ", ";
    if(item.ctrlType == ControlType::Device){
        ss << "deviceId: " << item.devGrpType   << ", ";
    }else if(item.ctrlType == ControlType::Group){
        ss << "groupId: " << item.devGrpType   << ", ";
    }else if(item.ctrlType == ControlType::Type){
        ss << "type: " << item.devGrpType   << ", ";
    }
    ss << "param: " << item.param;

    std::cout << ss.str() << std::endl;
}


void test(){
    string voiceData = "打开厨房的设备100";
    std::cout << "voiceData: " << voiceData << std::endl;
    ParsedItem parsedItem;

    //确定位置
    for(auto& elem :roomVec){
        smatch sm;
        if(regex_search(voiceData, sm, regex(elem))){
            parsedItem.roomName = elem;
            voiceData.erase(sm.position(), sm.length());
            break;
        }
    }
    std::cout << voiceData << std::endl;

    //确定动作
    for(auto& a2cElem : action2CodeMap){
        smatch sm;
        if(regex_match(voiceData, sm, regex(a2cElem.first))){
            ActionCode code = a2cElem.second;
            for(auto& c2cgElem : code2CaptureGrpMap){
                if(c2cgElem.first == code){
                    for(auto& groupIndex : c2cgElem.second){
                        voiceData.erase(sm.position(groupIndex), sm.length(groupIndex));
                    }
                    parsedItem.actionCode = code;
                    break;
                }
            }
        }
    }
    std::cout << voiceData << std::endl;

    //确定设备、组、类型


    //确定参数
    smatch sm;
    if(regex_match(voiceData, sm, regex("[^0-9]*(\\d*)[^0-9]*"))){
        parsedItem.param = sm.str(1);
        voiceData.erase(sm.position(1), sm.length(1));
    }
    std::cout << voiceData << std::endl;
    printParsedItem(parsedItem);
}


int main(int argc, char* argv[]){
   string str = "10.1.1.1:bl_site";
   if(regex_match(str, regex(".*:ble_site"))){
       std::cout << "match..." << std::endl;
   }
    return 0;
}




















