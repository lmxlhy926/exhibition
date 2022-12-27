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


int main(int argc, char* argv[]){
    string str = "hello";
    smatch sm;
    if(regex_search(str, sm, regex(""))){
        std::cout << "sssss" << std::endl;
    }

    return 0;
}




















