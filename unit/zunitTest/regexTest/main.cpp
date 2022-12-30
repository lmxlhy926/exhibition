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

std::vector<string> roomVec{
    "客厅", "厨房"
};

//std::map<string, Json::>
//
//std::vector<std::vector<string>> atciton{
//        {"打开"},
//        {"调到", "调为"}
//};


int main(int argc, char* argv[]){
    string voiceData = "打开厨房的设备1";

    string roomMatchExpression = "(.*)(厨房|客厅)(.*)";
    regex rg(roomMatchExpression);
    smatch sm;
    if(regex_match(voiceData, sm, rg)){
        std::cout << "size: " << sm.size() << std::endl;
        std::cout << "position: " << sm.position(2)  << ", length: " << sm.length(2) << std::endl;
    }
    voiceData.erase(6, 6);
    std::cout << "voiceData: " << voiceData << std::endl;

    return 0;
}




















