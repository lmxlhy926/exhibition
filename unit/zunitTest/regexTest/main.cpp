//
// Created by 78472 on 2022/12/11.
//

#include <regex>
#include <iostream>

using namespace std;

//开关灯
regex voice_expr_power_on("(.*)(灯)(.*)(开)(.*)");
regex voice_expr_power_on_1("(.*)(开)(.*)(灯)(.*)");
regex voice_expr_power_off("(.*)(灯)(.*)(关)(.*)");
regex voice_expr_power_off_1("(.*)(关)(.*)(灯)(.*)");



int main(int argc, char* argv[]){
    string str = "打开所有的灯";
    smatch sm;
    if(regex_match(str, sm, voice_expr_power_on_1)){

//        1. 设备名称
//        2. 组名称
//        3. 房间名称
//        4. 所有



        std::cout << sm.size() << std::endl;
    }



    return 0;
}




















