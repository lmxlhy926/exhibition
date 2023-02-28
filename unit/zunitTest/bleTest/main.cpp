
#include <iostream>
#include "common/httpUtil.h"
#include "qlibc/QData.h"
#include <regex>


void test(){
    string bleSiteName = "ble_light";
    string LocalIp = "192.168.58.116";
    int bleSitePort = 9001;

    SiteRecord::getInstance()->addSite(bleSiteName, LocalIp, bleSitePort);

    qlibc::QData openList, closeList, response;
    openList.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\open.json)");
    closeList.loadFromFile(R"(D:\bywg\project\exhibition\unit\bleTest\close.json)");

    for(int index = 0; index < 5; ++index){
        //依次打开客厅几个组的灯
        Json::ArrayIndex openListSize = openList.size();
        for(Json::ArrayIndex i = 0; i < openListSize; ++i){
            qlibc::QData controlData = openList.getArrayElement(i);
            SiteRecord::getInstance()->sendRequest2Site(bleSiteName, controlData, response);
            std::cout << "open: " << i << std::endl;
        }

        //依次关闭客厅几个组的灯
        Json::ArrayIndex closeListSize = closeList.size();
        for(Json::ArrayIndex i = 0; i < closeListSize; ++i){
            qlibc::QData controlData = closeList.getArrayElement(i);
            SiteRecord::getInstance()->sendRequest2Site(bleSiteName, controlData, response);
            std::cout << "close: " << i << std::endl;
        }
    }

    std::cout << "-------send complete---------" << std::endl;

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}


void groupControl(string group_id, string command_id, int command_para, string IP){
    qlibc::QData command, commandList, groupListItem, groupList, controlData;
    command.setString("command_id", command_id);
    command.setInt("command_para", command_para);
    command.setInt("transTime", 0);
    commandList.append(command);
    groupListItem.setString("group_id", group_id);
    groupListItem.putData("command_list", commandList);
    groupList.append(groupListItem);
    controlData.setString("service_id", "control_group");
    controlData.putData("request", qlibc::QData().putData("group_list", groupList));

    int PORT = 9001;
    qlibc::QData controlRes;
    httpUtil::sitePostRequest(IP, PORT, controlData, controlRes);
}


void groupControl_color(string group_id, string command_id, int commandParaLuminance, int commandParaColorTemperature, int transTime, string IP){
    qlibc::QData command, commandList, groupListItem, groupList, controlData;
    command.setString("command_id", command_id);
    command.setInt("command_para_luminance", commandParaLuminance);
    command.setInt("command_para_color_temperature", commandParaColorTemperature);
    command.setInt("transTime", transTime);
    commandList.append(command);
    groupListItem.setString("group_id", group_id);
    groupListItem.putData("command_list", commandList);
    groupList.append(groupListItem);
    controlData.setString("service_id", "control_group");
    controlData.putData("request", qlibc::QData().putData("group_list", groupList));

    int bleSitePort = 9001;
    qlibc::QData controlRes;
    httpUtil::sitePostRequest(IP, bleSitePort, controlData, controlRes);
}



void blink(){
    while(true){
        groupControl_color("FFFF", "luminance_color_temperature", 255, 6500, 0, "127.0.0.1");
//        groupControl_color("FFFF", "luminance_color_temperature", 255, 6500, 0, "10.1.1.120");
        sleep(4);
        groupControl_color("FFFF", "luminance_color_temperature", 0, 6500, 30, "127.0.0.1");
//        groupControl_color("FFFF", "luminance_color_temperature", 0, 6500, 30, "10.1.1.120");
        sleep(4);
        groupControl_color("FFFF", "luminance_color_temperature", 255, 2700, 0, "127.0.0.1");
//        groupControl_color("FFFF", "luminance_color_temperature", 255, 2700, 0, "10.1.1.120");
        sleep(4);
        groupControl_color("FFFF", "luminance_color_temperature", 0, 2700, 30, "127.0.0.1");
//        groupControl_color("FFFF", "luminance_color_temperature", 0, 2700, 30, "10.1.1.120");
        sleep(4);
    }
}

void blink1(){
    while(true){
        groupControl_color("FFFF", "luminance_color_temperature", 0, 2700, 30, "10.1.1.120");
        groupControl_color("FFFF", "luminance_color_temperature", 255, 6500, 30, "10.1.1.120");
    }
}



int main(int argc, char* argv[]){
    for(int i = 0; i < 20; ++i){
        groupControl_color("FFFF", "luminance_color_temperature", 0, 2700, 0, "10.1.1.120");
        groupControl_color("FFFF", "luminance_color_temperature", 255, 6500, 0, "10.1.1.120");
    }
    return 0;
}

