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

//获取扫描结果
int scan_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    if(requestBody.type() != Json::nullValue){
        //获取扫描结果
        qlibc::QData scanDeviceArray;
        lc.getScanedDevices(scanDeviceArray);

        //返回扫描结果
        qlibc::QData res, retData;
        res.putData("device_list", scanDeviceArray);
        retData.setInt("code", 0);
        retData.setString("error", "ok");
        retData.putData("response", res);
        response.set_content(retData.toJsonString(), "text/json");

        //结束扫描
        qlibc::QData scanEnd;
        scanEnd.setString("command", "scanEnd");
        lc.parse(scanEnd);
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


//绑定设备
int add_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
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
    LOG_INFO << "==>: " << requestBody.toJsonString();
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
    LOG_INFO << "==>: " << requestBody.toJsonString();
    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
            for(Json::ArrayIndex i = 0; i < deviceList.size(); ++i){
                qlibc::QData deviceItem = deviceList.getArrayElement(i);

                string device_id = deviceItem.getString("device_id");
                string device_address = SnAddressMap::getInstance()->deviceSn2Address(device_id);
                qlibc::QData command_list = deviceItem.getData("command_list");

                for(Json::ArrayIndex j = 0; j < command_list.size(); ++j){
                    qlibc::QData commandItem = command_list.getArrayElement(j);
                    string command_id = commandItem.getString("command_id");

                    qlibc::QData cmdData;
                    cmdData.setString("deviceAddress", device_address);
                    cmdData.setString("command", command_id);

                    if(command_id == POWER){
                        cmdData.setString("commandPara", commandItem.getString("command_para"));

                    }else if(command_id == LUMINANCE || command_id == COLORTEMPERATURE){
                        cmdData.setInt("commandPara", commandItem.getInt("command_para"));
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
int get_device_list_service_handler(const Request& request, Response& response){
    LOG_INFO << "==>: " << qlibc::QData(request.body).toJsonString();
    qlibc::QData postData;
    postData.setInt("code", 0);
    postData.setString("error", "ok");
    postData.putData("response", bleConfig::getInstance()->getDeviceListData());
    response.set_content(postData.toJsonString(), "text/json");
    return 0;
}

//获取设备状态
int get_device_state_service_handler(const Request& request, Response& response){
    LOG_INFO << "==>: " << qlibc::QData(request.body).toJsonString();
    qlibc::QData postData;
    postData.setInt("code", 0);
    postData.setString("error", "ok");
    postData.putData("response", bleConfig::getInstance()->getStatusListData());
    response.set_content(postData.toJsonString(), "text/json");
    return 0;
}


