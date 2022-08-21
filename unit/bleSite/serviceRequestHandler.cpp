//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "formatTrans/downBinaryCmd.h"
#include <regex>
#include "log/Logging.h"
#include "logic/logicControl.h"
#include "logic/snAddressMap.h"
#include "parameter.h"


static const nlohmann::json okResponse = {
        {"code", 0},
        {"error", "ok"},
        {"response",{}}
};

static const nlohmann::json errResponse = {
        {"code", 1},
        {"error", "request format is not correct"},
        {"response",{}}
};


int BleDevice_command_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData cmdData = requestBody.getData("request");
            lc.parse(cmdData);
        });
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}

int BleDevice_command_test_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        string command = requestBody.getData("request").getString("command");
        unsigned char buf[100]{};
        BinaryBuf binaryBuf(buf, 100);

        regex sep(" ");
        sregex_token_iterator p(command.cbegin(), command.cend(), sep, -1);
        sregex_token_iterator e;
        for(; p != e; ++p){
            binaryBuf.append(*p);
        }
        LOG_HLIGHT << "--->send: " << command;

        DownBinaryUtil::serialSend(buf, static_cast<int>(binaryBuf.size()));

        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }

    return 0;
}

//扫描设备
int scan_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        qlibc::QData scanDeviceArray;
        lc.getScanedDevices(scanDeviceArray);

        qlibc::QData res, retData;
        res.putData("device_list", scanDeviceArray);
        retData.setInt("code", 0);
        retData.setString("error", "ok");
        retData.putData("response", res);

        response.set_content(retData.toJsonString(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


//添加设备
int add_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
            qlibc::QData cmdData;
            cmdData.setString("command", "bind");
            cmdData.putData("device_list", deviceList);
            lc.parse(cmdData);
        });
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}

//删除设备
int del_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
            qlibc::QData cmdData;
            cmdData.setString("command", "unbind");
            cmdData.putData("device_list", deviceList);
            lc.parse(cmdData);
        });
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}

//控制设备
int control_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
            for(unsigned i = 0; i < deviceList.size(); ++i){
                string device_id = deviceList.getArrayElement(i).getString("device_id");
                string device_address = SnAddressMap::getInstance()->deviceSn2Address(device_id);

                qlibc::QData command_list = deviceList.getArrayElement(i).getData("command_list");
                for(int j = 0; j < command_list.size(); ++j){
                    string command_id = command_list.getArrayElement(j).getString("command_id");

                    qlibc::QData cmdData;
                    cmdData.setString("deviceAddress", device_address);
                    cmdData.setString("command", command_id);
                    if(command_id == POWER){
                        string command_para = command_list.getArrayElement(j).getString("command_para");
                        cmdData.setString("commandPara", command_para);

                    }else if(command_id == LUMINANCE){
                        int command_para = command_list.getArrayElement(j).getInt("command_para");
                        cmdData.setInt("commandPara", command_para);

                    }else if(command_id == COLORTEMPERATURE){
                        int command_para = command_list.getArrayElement(j).getInt("command_para");
                        cmdData.setInt("commandPara", command_para);
                    }

                    lc.parse(cmdData);
                }
            }
        });

        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


//获取设备列表
int get_device_list_service_handler(const Request& request, Response& response, LogicControl& lc){
    return 0;
}

//获取设备状态
int get_device_state_service_handler(const Request& request, Response& response, LogicControl& lc){
    return 0;
}


