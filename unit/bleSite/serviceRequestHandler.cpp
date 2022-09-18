//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "formatTrans/downUtil.h"
#include "formatTrans/bleConfig.h"
#include <regex>
#include "log/Logging.h"
#include "logic/logicControl.h"
#include "logic/snAddressMap.h"
#include "logic/groupAddressMap.h"
#include "common/httpUtil.h"
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


/*
 * 如果是单个设备控制：则需将设备mac转换为设备地址
 * 如果是组控制：因为组id就是组地址，所以无需转换
 */
void controlDevice(qlibc::QData& deviceList, LogicControl& lc){
    for(Json::ArrayIndex i = 0; i < deviceList.size(); ++i){
        qlibc::QData deviceItem = deviceList.getArrayElement(i);
        string device_id = deviceItem.getString("device_id");
        string group_id = deviceItem.getString("group_id");
        if(device_id.empty() && group_id.empty()){
            continue;
        }

        string address;
        if(!device_id.empty())
            address = SnAddressMap::getInstance()->deviceSn2Address(device_id);
        else
            address = group_id;

        if(address.empty())
            continue;

        qlibc::QData command_list = deviceItem.getData("command_list");
        for(Json::ArrayIndex j = 0; j < command_list.size(); ++j){
            qlibc::QData commandItem = command_list.getArrayElement(j);
            string command_id = commandItem.getString("command_id");

            qlibc::QData cmdData;
            cmdData.setString("address", address);
            cmdData.setString("command", command_id);

            if(command_id == POWER){
                cmdData.setString("commandPara", commandItem.getString("command_para"));

            }else if(command_id == LUMINANCE || command_id == COLORTEMPERATURE){
                cmdData.setInt("commandPara", commandItem.getInt("command_para"));
            }

            lc.parse(cmdData);
        }
    }
}


//获取扫描结果
int scan_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    if(requestBody.type() != Json::nullValue){
        //获取扫描结果
        qlibc::QData scanDeviceArray;
        qlibc::QData emptyParam;
        lc.getScanedDevices(scanDeviceArray, emptyParam);

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
            qlibc::QData cmdData;
            cmdData.setString("command", ADD_DEVICE);
            cmdData.putData("param", requestBody.getData("request"));
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
            size_t deviceListSize = deviceList.size();
            for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
                qlibc::QData cmdData;
                cmdData.setString("command", "unbind");
                cmdData.putData("deviceSn", deviceList.getArrayElement(i));
                lc.parse(cmdData);
            }
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
            controlDevice(deviceList, lc);
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
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();

    qlibc::QData statusItems;
    qlibc::QData statusListData = bleConfig::getInstance()->getStatusListData().getData("device_list");
    ssize_t statusListSize = statusListData.size();
    qlibc::QData requestDeviceList = requestBody.getData("request").getData("device_list");
    ssize_t requestDeviceListSize = requestDeviceList.size();

    for(ssize_t i = 0; i < requestDeviceListSize; ++i){
        string device_id = requestDeviceList.getArrayElement(i).getString("device_id");
        for(ssize_t j = 0; j < statusListSize; ++j){
            qlibc::QData item = statusListData.getArrayElement(j);
            if(item.getString("device_id") == device_id){
                statusItems.append(item);
                break;
            }
        }
    }

    qlibc::QData deviceList;
    if(requestDeviceListSize == 0){
        deviceList.putData("device_list", statusListData);
    }else{
        deviceList.putData("device_list", statusItems);
    }

    qlibc::QData postData;
    postData.setInt("code", 0);
    postData.setString("error", "ok");
    postData.putData("response", deviceList);
    response.set_content(postData.toJsonString(), "text/json");
    return 0;
}

int BleDevice_command_test_service_handler(const Request& request, Response& response){
#if 0
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        string command = requestBody.getData("request").getString("command");
        LOG_HLIGHT << "--->received command: " << command;

        unsigned char buf[100]{};
        BinaryBuf binaryBuf(buf, 100);
        regex sep(" ");
        sregex_token_iterator p(command.cbegin(), command.cend(), sep, -1);
        sregex_token_iterator e;
        for(; p != e; ++p){
            binaryBuf.append(*p);
        }

        DownBinaryUtil::serialSend(buf, static_cast<int>(binaryBuf.size()));

        response.set_content(okResponse.dump(), "text/json");

    }else{
        response.set_content(errResponse.dump(), "text/json");
    }

    return 0;
#endif
    return 0;
}


//设备分组
int group_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData cmdData;
            cmdData.setString("command", "group");
            cmdData.setString("group_name", requestBody.getData("request").getString("group_name"));
            cmdData.putData("device_list", requestBody.getData("request").getData("device_list"));
            lc.parse(cmdData);
        });

        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


//创建分组
int create_group_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    string groupName = requestBody.getData("request").getString("group_name");
    GroupAddressMap::getInstance()->createGroup(groupName);

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//删除分组
int delete_group_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    bleConfig::getInstance()->enqueue([requestBody, &lc]{
        string group_id = requestBody.getData("request").getString("group_id");
        qlibc::QData device_list;
        qlibc::QData groupList = GroupAddressMap::getInstance()->getGroupList().getData("group_list");
        size_t groupListSize = groupList.size();
        for(Json::ArrayIndex i = 0; i < groupListSize; ++i){
            qlibc::QData groupItem = groupList.getArrayElement(i);
            if(groupItem.getString("group_id") == group_id){
                device_list = groupItem.getData("device_list");
            }
        }

        size_t deviceListSize = device_list.size();
        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            qlibc::QData cmdData;
            cmdData.setString("command", "delDeviceFromGroup");
            cmdData.setString("group_id", group_id);
            cmdData.setString("deviceSn", device_list.getArrayElement(i).asValue().asString());
            lc.parse(cmdData);
        }

        GroupAddressMap::getInstance()->deleGroup(group_id);
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//重命名分组
int rename_group_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    string groupId = requestBody.getData("request").getString("group_id");
    string groupName = requestBody.getData("request").getString("group_name");
    GroupAddressMap::getInstance()->reNameGroup(groupId, groupName);

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//添加设备进入分组
int addDevice2Group_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    bleConfig::getInstance()->enqueue([requestBody, &lc]{
        string group_id = requestBody.getData("request").getString("group_id");
        if(!GroupAddressMap::getInstance()->isGroupExist(group_id)){
            LOG_RED  << "group " << group_id << " is not exist....";
            return;
        }
        qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
        size_t deviceListSize = deviceList.size();
        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            qlibc::QData cmdData;
            cmdData.setString("command", "addDevice2Group");
            cmdData.setString("group_id", group_id);
            cmdData.putData("deviceSn", deviceList.getArrayElement(i));

            cmdData.setString("model_name", POWER);
            lc.parse(cmdData);

            cmdData.setString("model_name", LUMINANCE);
            lc.parse(cmdData);

            cmdData.setString("model_name", COLORTEMPERATURE);
            lc.parse(cmdData);
        }
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//从分组移除设备
int removeDeviceFromGroup_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    bleConfig::getInstance()->enqueue([requestBody, &lc]{
        string group_id = requestBody.getData("request").getString("group_id");
        if(!GroupAddressMap::getInstance()->isGroupExist(group_id)){
            LOG_RED << "group " << group_id << " is not exist....";
            return;
        }
        qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
        size_t deviceListSize = deviceList.size();
        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            qlibc::QData cmdData;
            cmdData.setString("command", "delDeviceFromGroup");
            cmdData.setString("group_id", group_id);
            cmdData.putData("deviceSn", deviceList.getArrayElement(i));

            cmdData.setString("model_name", POWER);
            lc.parse(cmdData);

            cmdData.setString("model_name", LUMINANCE);
            lc.parse(cmdData);

            cmdData.setString("model_name", COLORTEMPERATURE);
            lc.parse(cmdData);
        }
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//控制分组
int control_group_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();

    if(requestBody.type() != Json::nullValue){
        bleConfig::getInstance()->enqueue([requestBody, &lc]{
            qlibc::QData groupList = requestBody.getData("request").getData("group_list");
            controlDevice(groupList, lc);
        });

        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


//获取分组列表
int getGroupList_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    qlibc::QData retData;
    retData.setInt("code", 0);
    retData.setString("error", "ok");
    retData.putData("response", GroupAddressMap::getInstance()->getGroupList());

    response.set_content(retData.toJsonString(), "text/json");
    return 0;
}


void updateDeviceList(const Request& request){
    //获取白名单列表
    LOG_INFO << "updateDeviceList: " << request.body;
    qlibc::QData whiteListRequest, whiteListResponse;
    whiteListRequest.setString("service_id", "whiteListRequest");
    whiteListRequest.putData("request", qlibc::QData());
    SiteRecord::getInstance()->sendRequest2Site(ConfigSiteName, whiteListRequest, whiteListResponse);

    if(whiteListResponse.getInt("code") == 0){
        qlibc::QData configWhiteListDevices = whiteListResponse.getData("response").getData("info").getData("devices");
        size_t configWhiteListDevicesSize = configWhiteListDevices.size();
        if(0 == configWhiteListDevicesSize){
            return;
        }

        qlibc::QData lightDevices;
        for(int i = 0; i < configWhiteListDevicesSize; ++i){
            qlibc::QData item = configWhiteListDevices.getArrayElement(i);
            string category_code = item.getString("category_code");
            if(category_code == "LIGHT" || category_code == "light"){
                lightDevices.append(item);
            }
        }
        ssize_t lightDeviceSize = lightDevices.size();
        if(0 == lightDeviceSize){
            return;
        }

        qlibc::QData deviceList = bleConfig::getInstance()->getDeviceListData().getData("device_list");
        size_t deviceListSize = deviceList.size();
        qlibc::QData newDeviceList;

        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            qlibc::QData deviceItem =  deviceList.getArrayElement(i);

            for(Json::ArrayIndex j = 0; j < lightDeviceSize; ++j){
                qlibc::QData configDevice = lightDevices.getArrayElement(j);

                if(configDevice.getString("device_sn") == deviceItem.getString("device_id")){
                    qlibc::QData location;
                    location.setString("room_name", configDevice.getString("room_name"));
                    location.setString("room_no", configDevice.getString("room_no"));
                    deviceItem.setString("device_name", configDevice.getString("device_name"));
                    deviceItem.setString("device_brand", configDevice.getString("device_brand"));
                    deviceItem.putData("location", location);

                    newDeviceList.append(deviceItem);
                    break;
                }

                if(j == configWhiteListDevicesSize -1){
                    newDeviceList.append(deviceItem);
                }
            }
        }

        //存储设备列表
        qlibc::QData saveData;
        saveData.putData("device_list", newDeviceList);
        bleConfig::getInstance()->saveDeviceListData(saveData);
    }

}
