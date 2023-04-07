//
// Created by 78472 on 2022/5/15.
//

#include "parameter.h"
#include "qlibc/QData.h"
#include "common/httpUtil.h"
#include "serial/telinkDongle.h"
#include "sourceManage/snAddressMap.h"
#include "sourceManage/groupAddressMap.h"
#include "downCmd/logicControl.h"
#include "log/Logging.h"
#include "sourceManage/bleConfig.h"
#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include <regex>
#include "../sourceManage/util.h"


static const nlohmann::json okResponse = {
        {"code", 0},
        {"error", "ok"},
        {"response",{}}
};

static const nlohmann::json errResponse = {
        {"code", 1},
        {"error", "failed"},
        {"response",{}}
};

/*
 * 单个设备控制和组设备控制，控制指令是相同的，区分是地址是设备地址还是组地址
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

        bool isGroup{false};
        string address;
        if(!device_id.empty())
            address = SnAddressMap::getInstance()->deviceSn2Address(device_id);
        else{
            address = group_id;
            isGroup = true;
        }

        if(address.empty())
            continue;

        qlibc::QData command_list = deviceItem.getData("command_list");
        for(Json::ArrayIndex j = 0; j < command_list.size(); ++j){
            qlibc::QData commandItem = command_list.getArrayElement(j);
            string command_id = commandItem.getString("command_id");
            int transTime = commandItem.getInt("transTime");

            qlibc::QData cmdData;
            cmdData.setString("address", address);
            cmdData.setString("command", command_id);
            cmdData.setInt("transTime", transTime);

            if(command_id == POWER){    //开关
                string powerOnOff = commandItem.getString("command_para");
                cmdData.setString("commandPara", powerOnOff);
                if(isGroup){
                    if(powerOnOff == "on"){
                        bleConfig::getInstance()->powerOn(address);
                    }else if(powerOnOff == "off"){
                        bleConfig::getInstance()->powerOff(address);
                    }
                }

            }else if(command_id == LUMINANCE){  //亮度
                int luminance = commandItem.getInt("command_para");
                if(luminance > 255){
                    luminance = 255;
                }else if(luminance < 0){
                    luminance = 0;
                }
                cmdData.setInt("commandPara", luminance);
                if(isGroup){
                    bleConfig::getInstance()->storeGroupluminance(address, luminance);
                }

            }else if(command_id == COLORTEMPERATURE){   //色温
                int colorTemperature = commandItem.getInt("command_para");
                if(colorTemperature > 6500){
                    colorTemperature = 6500;
                }else if(colorTemperature < 2700){
                    colorTemperature = 2700;
                }
                cmdData.setInt("commandPara", colorTemperature);
                if(isGroup){
                    bleConfig::getInstance()->storeGroupColorTemperature(address, colorTemperature);
                }

            }else if(command_id == LUMINANCERELATIVE){  //相对亮度
                if(!isGroup){
                    cmdData.setInt("commandPara", commandItem.getInt("command_para"));
                }else{
                    int relative = commandItem.getInt("command_para");
                    int setLuminance{};
                    if (bleConfig::getInstance()->getGroupLuminanceColorTemperature(address)["luminance"].empty()) {
                        setLuminance = 128;
                    } else {
                        int currentLuminance = bleConfig::getInstance()->getGroupLuminanceColorTemperature(address)["luminance"].asInt();
                        setLuminance = static_cast<int>(currentLuminance + 2.55 * relative);
                        if (setLuminance > 255) {
                            setLuminance = 255;
                        } else if (setLuminance < 0) {
                            setLuminance = 0;
                        }
                    }
                    cmdData.setString("command", LUMINANCE);
                    cmdData.setInt("commandPara", setLuminance);
                    bleConfig::getInstance()->storeGroupluminance(address, setLuminance);
                }

            }else if(command_id == COLORTEMPERATURERELATIVE){   //相对色温
                if(!isGroup){
                    cmdData.setInt("commandPara", commandItem.getInt("command_para"));
                }else{
                    int relative = commandItem.getInt("command_para");
                    int setColorTemperature{};
                    if (bleConfig::getInstance()->getGroupLuminanceColorTemperature(address)["temperature"].empty()) {
                        setColorTemperature = 4600;
                    } else {
                        int currentColorTemperature = bleConfig::getInstance()->getGroupLuminanceColorTemperature(address)["temperature"].asInt();
                        setColorTemperature = currentColorTemperature + 38 * relative;
                        if (setColorTemperature > 6500) {
                            setColorTemperature = 6500;
                        } else if (setColorTemperature < 2700) {
                            setColorTemperature = 2700;
                        }
                    }
                    cmdData.setString("command", COLORTEMPERATURE);
                    cmdData.setInt("commandPara", setColorTemperature);
                    bleConfig::getInstance()->storeGroupColorTemperature(address, setColorTemperature);
                }

            }else if(command_id == LUMINANCECOLORTEMPERATURE){  //亮度、色温联合控制
                int luminance = commandItem.getInt("command_para_luminance");
                if(luminance > 255){
                    luminance = 255;
                }else if(luminance < 0){
                    luminance = 0;
                }

                int colorTemperature = commandItem.getInt("command_para_color_temperature");
                if(colorTemperature > 6500){
                    colorTemperature = 6500;
                }else if(colorTemperature < 2700){
                    colorTemperature = 2700;
                }

                cmdData.setInt("commandParaLuminance", luminance);
                cmdData.setInt("commandParaColorTemperature", colorTemperature);

                if(isGroup){
                    bleConfig::getInstance()->storeGroupluminance(address, luminance);
                    bleConfig::getInstance()->storeGroupColorTemperature(address, colorTemperature);
                }

                //打印控制信息
                qlibc::QData groupList = bleConfig::getInstance()->getGroupListData();
                Json::Value::Members groupAddressVec = groupList.getMemberNames();
                for(auto& key: groupAddressVec){
                    if(key == address){
                        string groupName = groupList.getData(key).getString("group_name");
                        LOG_YELLOW << "===>" << groupName << ": " << commandItem.getInt("command_para_luminance") << ", "
                        << commandItem.getInt("command_para_color_temperature");
                        break;
                    }
                }

            }else if(command_id == MODECONFIG){   //模式
                cmdData.setInt("commandPara", commandItem.getInt("command_para"));

            }

            lc.parse(cmdData);
        }
    }
}

int reset_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    LOG_INFO << "reset_device_service_handler: " << qlibc::QData(request.body).toJsonString();
    string command = "E9FF02";
    string serialName = bleConfig::getInstance()->getSerialData().getString("serial");
    TelinkDongle* telinkDonglePtr = TelinkDongle::getInstance(serialName);
    telinkDonglePtr->write2Seria(command);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//获取扫描结果
int scan_device_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    if(requestBody.type() != Json::nullValue){
        //发送扫描指令，获取扫描结果
        qlibc::QData scanDeviceArray;
        qlibc::QData param = requestBody.getData("request");
        lc.getScanedDevices(scanDeviceArray, param);

        //提取sn列表
        qlibc::QData deviceId_list;
        Json::ArrayIndex scanDeviceArraySize = scanDeviceArray.size();
        for(Json::ArrayIndex i = 0; i < scanDeviceArraySize; ++i){
            deviceId_list.append(scanDeviceArray.getArrayElement(i).getString("deviceSn"));
        }

        //返回扫描结果
        qlibc::QData res, retData;
        res.putData("device_list", scanDeviceArray);
        res.putData("deviceId_list", deviceId_list);
        retData.setInt("code", 0);
        retData.setString("error", "ok");
        retData.putData("response", res);
        response.set_content(retData.toJsonString(), "text/json");

        //发送结束扫描指令
        qlibc::QData scanEndCmd;
        scanEndCmd.setString("command", SCANEND);
        lc.parse(scanEndCmd);
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


int scanDevice_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    bleConfig::getInstance()->enqueue([requestBody, &lc]{
        qlibc::QData scanDeviceArray;
        qlibc::QData param = requestBody.getData("request");
        lc.getScanedDevices(scanDeviceArray, param);

        //发送结束扫描指令
        qlibc::QData scanEndCmd;
        scanEndCmd.setString("command", SCANEND);
        lc.parse(scanEndCmd);
    });
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//添加设备：扫描、绑定
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
                cmdData.setString("command", UNBIND);
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
    LOG_INFO << "==>control_device_service_handler: " << requestBody.toJsonString();
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

//按照指定房间获取设备列表
int get_device_list_byRoomName_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    string room_no = requestBody.getData("request").getString("room_no");
    qlibc::QData device_list = bleConfig::getInstance()->getDeviceListData().getData("device_list");
    qlibc::QData deviceList2show;
    for(Json::ArrayIndex i = 0; i < device_list.size(); ++i){
        qlibc::QData item = device_list.getArrayElement(i);
        if(item.getData("location").getString("room_no") == room_no){
           deviceList2show.append(item);
        }
    }

    qlibc::QData responseData;
    responseData.putData("device_list", deviceList2show);

    qlibc::QData postData;
    postData.setInt("code", 0);
    postData.setString("error", "ok");
    postData.putData("response", responseData);
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

//存储设备列表
int save_deviceList_service_handler(const Request& request, Response& response){
    qlibc::QData requestData(request.body);
    LOG_INFO << "==>: " << requestData.toJsonString();
    qlibc::QData deviceList = requestData.getData("request");
    bleConfig::getInstance()->saveDeviceListData(deviceList);
    response.set_content(okResponse.dump(), "text/json");
    return 0;
}

int BleDevice_command_test_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    if(requestBody.type() != Json::nullValue){
        string command = requestBody.getData("request").getString("command");
        string commandWithoutSpace;
        regex sep("[ ]+");
        sregex_token_iterator end;
        sregex_token_iterator pos(command.cbegin(), command.cend(), sep, {-1});
        for(; pos != end; ++pos){
            commandWithoutSpace += pos->str();
        }
        LOG_HLIGHT << "--->received command: " << commandWithoutSpace;

        string serialName = bleConfig::getInstance()->getSerialData().getString("serial");
        TelinkDongle* telinkDonglePtr = TelinkDongle::getInstance(serialName);
        telinkDonglePtr->write2Seria(commandWithoutSpace);

        response.set_content(okResponse.dump(), "text/json");

    }else{
        response.set_content(errResponse.dump(), "text/json");
    }

    return 0;
}

//修改设备信息
int device_config_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "device_config_service_handler: " << requestBody.toJsonString();

    qlibc::QData deviceList= bleConfig::getInstance()->getDeviceListData().getData("device_list");
    Json::ArrayIndex deviceListSize = deviceList.size();
    qlibc::QData configList = requestBody.getData("request").getData("device_list");
    Json::ArrayIndex configListSize = configList.size();

    qlibc::QData newDeviceList;
    for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
        qlibc::QData deviceItem = deviceList.getArrayElement(i);
        for(int j = 0; j < configListSize; ++j){
            qlibc::QData configItem = configList.getArrayElement(j);
            if(deviceItem.getString("device_id") == configItem.getString("device_id")){
                if(!configItem.getString("device_name").empty())
                    deviceItem.setString("device_name", configItem.getString("device_name"));
                deviceItem.putData("location", configItem.getData("location"));
                newDeviceList.append(deviceItem);
                break;
            }
            if(j == configListSize - 1){
                newDeviceList.append(deviceItem);
            }
        }
    }
    bleConfig::getInstance()->saveDeviceListData(qlibc::QData().putData("device_list", newDeviceList));

    //通知设备管理站点更新设备列表
    util::updateDeviceList();

    response.set_content(okResponse.dump(), "text/json");
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


void addRemoveGroupControl(const char* command, const string& group_id, const string& deviceSn, LogicControl& lc){
    qlibc::QData cmdData;
    cmdData.setString("command", string(command));
    cmdData.setString("group_id", group_id);
    cmdData.setString("deviceSn", deviceSn);

    cmdData.setString("model_name", POWER);
    lc.parse(cmdData);

    cmdData.setString("model_name", LUMINANCE);
    lc.parse(cmdData);

    cmdData.setString("model_name", COLORTEMPERATURE);
    lc.parse(cmdData);

    cmdData.setString("model_name", LUMINANCECOLORTEMPERATURE);
    lc.parse(cmdData);
}


//创建分组
int create_group_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    qlibc::QData property = requestBody.getData("request");
    string groupId = GroupAddressMap::getInstance()->createGroup(property);

    //更新分组列表
    util::updateGroupList();

    qlibc::QData res;
    res.setInt("code", 0);
    res.setString("error", "success");
    res.putData("response", qlibc::QData().setString("group_id", groupId));

    response.set_content(res.toJsonString(), "text/json");
    return 0;
}

//删除分组
int delete_group_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    bleConfig::getInstance()->enqueue([requestBody, &lc]{
        //依据组名，提取组下面的设备列表
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

        //将设备列表中的设备从分组中解除
        size_t deviceListSize = device_list.size();
        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            string deviceSn = device_list.getArrayElement(i).asValue().asString();
            addRemoveGroupControl(DelDeviceFromGroup, group_id, deviceSn, lc);
        }

        //删除分组
        GroupAddressMap::getInstance()->deleGroup(group_id);

        //更新分组列表
        util::updateGroupList();
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//重命名分组
int rename_group_service_handler(const Request& request, Response& response){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    string groupId = requestBody.getData("request").getString("group_id");
    qlibc::QData property = requestBody.getData("request");
    bool ret = GroupAddressMap::getInstance()->reNameGroup(groupId, property);

    //更新分组列表
    util::updateGroupList();

    if(ret){
        response.set_content(okResponse.dump(), "text/json");
    }else{
        response.set_content(errResponse.dump(), "text/json");
    }
    return 0;
}


//添加设备进入分组
int addDevice2Group_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    string group_id = requestBody.getData("request").getString("group_id");
    if(!GroupAddressMap::getInstance()->isGroupExist(group_id)){
        LOG_RED  << "group " << group_id << " is not exist....";
        response.set_content(errResponse.dump(), "text/json");
        return 0;
    }

    bleConfig::getInstance()->enqueue([requestBody, group_id, &lc]{
        qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
        size_t deviceListSize = deviceList.size();
        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            string deviceSn = deviceList.getArrayElement(i).asValue().asString();
            addRemoveGroupControl(AddDevice2Group, group_id, deviceSn, lc);
        }
        //更新分组列表
        util::updateGroupList();
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


int groupByRoomname_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    string room_no = requestBody.getData("request").getString("room_no");
    string group_id = requestBody.getData("request").getString("group_id");
    if(group_id.empty()){
        LOG_RED << "THE GROUP <" << group_id << "> IS NOT EXIST....";
        response.set_content(errResponse.dump(), "text/json");
        return 0;
    }

    bleConfig::getInstance()->enqueue([requestBody, room_no, group_id, &lc]{
        //根据房间名获取设备列表
        qlibc::QData device_list  = bleConfig::getInstance()->getDeviceListData().getData("device_list");
        for(Json::ArrayIndex i = 0; i < device_list.size(); ++i){
            qlibc::QData item = device_list.getArrayElement(i);
            if(item.getData("location").getString("room_no") == room_no){
                string deviceSn = item.getString("device_id");
                addRemoveGroupControl(AddDevice2Group, group_id, deviceSn, lc);
            }
        }
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//从分组移除设备
int removeDeviceFromGroup_service_handler(const Request& request, Response& response, LogicControl& lc){
    qlibc::QData requestBody(request.body);
    LOG_INFO << "==>: " << requestBody.toJsonString();
    string group_id = requestBody.getData("request").getString("group_id");
    if(!GroupAddressMap::getInstance()->isGroupExist(group_id)){
        LOG_RED << "group " << group_id << " is not exist....";
        response.set_content(errResponse.dump(), "text/json");
        return 0;
    }

    bleConfig::getInstance()->enqueue([requestBody, group_id, &lc]{
        qlibc::QData deviceList = requestBody.getData("request").getData("device_list");
        size_t deviceListSize = deviceList.size();
        for(Json::ArrayIndex i = 0; i < deviceListSize; ++i){
            string deviceSn = deviceList.getArrayElement(i).asValue().asString();
            addRemoveGroupControl(DelDeviceFromGroup, group_id, deviceSn, lc);
        }
        //更新分组列表
        util::updateGroupList();
    });

    response.set_content(okResponse.dump(), "text/json");
    return 0;
}


//控制分组
int control_group_service_handler(const Request& request, Response& response, LogicControl& lc){
    LOG_INFO << "==>control_group_service_handler: " << qlibc::QData(request.body).toJsonString();
    qlibc::QData requestBody(request.body);
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

