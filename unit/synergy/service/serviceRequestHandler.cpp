//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "qlibc/QData.h"
#include "../param.h"
#include "common/httpUtil.h"
#include "../control/downCommand.h"
#include "log/Logging.h"
#include "../sourceManage/deviceManager.h"
#include "../sourceManage/groupManager.h"
#include "../control/sceneCommand.h"
#include "../voiceControl/voiceMatch.h"
#include <vector>
#include <regex>

using namespace servicesite;

namespace synergy {

    std::vector<string> sceneVec = {
      "constantBrightnessModeStart",
      "constantBrightnessModeStop",
      "constantBrightnessModeFlag",

      "comfortableDinnerModeStart",
      "comfortableDinnerModeStop",

      "readModeStart",
      "readModeStop",

      "cookModeStart",
      "cookModeStop",

      "enterHouseholdModeStart",
      "enterHouseholdNoPersonModeStart",

      "viewingSceneStart",
      "viewingSceneStop"
    };

    static const nlohmann::json okResponse = {
            {"code",     0},
            {"error",    "ok"},
            {"response", {}}
    };

    static const nlohmann::json errResponse = {
            {"code",     1},
            {"error",    "failed"},
            {"response", {}}
    };


    //构造关闭特定分组的控制指令
    qlibc::QData buildControlCmd(string group_id, int luminance, int colorTemperature){
        qlibc::QData command, commandList, deviceItem, deviceList;
        command.setString("command_id", "luminance_color_temperature");
        command.setInt("command_para_luminance", luminance);
        command.setInt("command_para_color_temperature", colorTemperature);
        command.setInt("transTime", 0);
        commandList.append(command);

        deviceItem.setString("group_id", group_id);
        deviceItem.putData("command_list", commandList);

        qlibc::QData requestData;
        requestData.setString("service_id", "control_group");
        requestData.putData("request", qlibc::QData().putData("group_list", qlibc::QData().append(deviceItem)));
        return requestData;
    }

    bool isBleMeshSiteOnline(){
        qlibc::QData requestData, responseData;
        requestData.setString("service_id", "get_message_list");
        requestData.putData("request", qlibc::QData());
        return httpUtil::sitePostRequest("127.0.0.1", 9001, requestData, responseData);
    }

    bool bleGroupRegister2Cloud(){
        qlibc::QData groupList = GroupManager::getInstance()->getBleGroupList();
        Json::ArrayIndex size = groupList.size();
        qlibc::QData deviceList;
        for(Json::ArrayIndex i = 0; i < size; ++i){
            qlibc::QData item = groupList.getArrayElement(i);
            qlibc::QData data;
            data.setString("categoryCode", "LIGHT");
            data.setString("deviceCode", item.getString("group_uid"));
            data.setString("deviceDid", item.getString("group_uid"));
            data.setString("deviceSn", item.getString("group_uid"));
            data.setString("deviceName", item.getString("group_name"));
            data.setString("deviceDesc", item.getString("group_name"));
            data.setString("deviceVender", "changhong");
            data.setString("productNickname", item.getString("group_name"));
            data.setString("productType", "LIGHT_SWITCH");
            data.setString("roomNo", item.getData("location").getString("room_no"));
            data.setString("roomType", item.getData("location").getString("room_name"));
            data.setString("roomName", item.getData("location").getString("room_name"));
            data.setInt("deviceSource", 1);
            data.setInt("isGateway", 0);
            deviceList.append(data);
        }
        qlibc::QData requestData, responseData;
        requestData.setString("service_id", "postDeviceList");
        requestData.putData("request", qlibc::QData().putData("deviceList", deviceList));
        LOG_INFO << "requestData: " << requestData.toJsonString(true);

        if(httpUtil::sitePostRequest("127.0.0.1", 9006, requestData, responseData)){
           return true;
        }

        return false;
    }

    int bleDeviceRegister_service_handler(const Request& request, Response& response){
        LOG_INFO << "bleDeviceRegister_service_handler: " << qlibc::QData(request.body).toJsonString();
        if(bleGroupRegister2Cloud()){
            response.set_content(okResponse.dump(), "text/json");
        }else{
            response.set_content(errResponse.dump(), "text/json");
        }
        return 0;
    }

    //软服针对样板间下发给mesh站点的控制指令
    void lightControlBleSite(const string& area){
        if(area == "all"){      //控制所有灯
            qlibc::QData requestData = buildControlCmd("FFFF", 0, 6500);
            qlibc::QData responseData;
            std::set<string> siteName = SiteRecord::getInstance()->getSiteName();
            for(auto& elem : siteName){
                smatch sm;
                if(regex_match(elem, sm, regex("(.*):(.*)"))){
                    if(sm.str(2) == BleSiteID){   //只控制蓝牙设备
                        LOG_GREEN << elem << " requestData: " << requestData.toJsonString();
                        SiteRecord::getInstance()->sendRequest2Site(elem, requestData, responseData);
                        LOG_BLUE << elem << " responseData: " << responseData.toJsonString();
                    }
                }
            }
            return;
        }

        //获取组列表
        qlibc::QData groupList = GroupManager::getInstance()->getAllGroupList();
        Json::ArrayIndex groupListSize = groupList.size();
        for(Json::ArrayIndex i = 0; i < groupListSize; ++i){
            qlibc::QData item = groupList.getArrayElement(i);
            string item_room_no = item.getData("location").getString("room_no");
            string item_sourceSite = item.getString("sourceSite");
            if(item.getBool("areaFullGroup") && area == item_room_no){   //控制某个指定房间的灯
                string group_id = item.getString("group_id");
                qlibc::QData requestData = buildControlCmd(group_id, 0, 6500);
                qlibc::QData responseData;
                smatch sm;
                if(regex_match(item_sourceSite, sm, regex("(.*):(.*)"))){
                    if(sm.str(2) == BleSiteID){
                        LOG_GREEN << item_sourceSite << " requestData: " << requestData.toJsonString();
                        SiteRecord::getInstance()->sendRequest2Site(item_sourceSite, requestData, responseData);
                        LOG_BLUE << item_sourceSite << " responseData: " << responseData.toJsonString();
                        break;
                    }
                }
            }
        }
    }

    int cloudCommand_service_handler(const Request& request, Response& response){
        qlibc::QData requestData(request.body);
        LOG_INFO << "cloudCommand_service_handler: " << requestData.toJsonString();
        string action = requestData.getData("request").getString("action");
        string code = requestData.getData("request").getString("code");
        string area = requestData.getData("request").getData("inParams").getString("area");

        //场景指令转换后，发送到场景站点
        for(auto& elem : sceneVec){
            if(action == elem){     //通过action判断是否是场景指令, 如果是，转换指令并发送给场景站点
                //发布场景指令消息
                qlibc::QData publishData;
                publishData.setString("message_id", Scene_Msg_MessageID);
                publishData.putData("content", requestData.getData("request"));
                ServiceSiteManager::getInstance()->publishMessage(Scene_Msg_MessageID, publishData.toJsonString());
                LOG_INFO << "publish Scene_Msg_MessageID: " << publishData.toJsonString();

                //进行格式转换，请求场景站点接口
                qlibc::QData siteResponse;
                SceneCommand sc(requestData);
                if(!sc.sendCmd(siteResponse)){
                    response.set_content(errResponse.dump(), "text/json");
                    LOG_RED << "==>sceneCommand send failed, sitResponse: " << siteResponse.toJsonString();
                }else{
                    LOG_BLUE << "==>sceneCommand send sucessfully, sitResponse: " << siteResponse.toJsonString();
                    response.set_content(siteResponse.toJsonString(), "text/json");
                }
                return 0;
            }
        }

        /*
         * 蓝牙灯和zigbee灯只会存在一种，优先判断是否是蓝牙灯的控制
         * 控制蓝牙分组，只控制关闭
         */
        if(code == "light"){
            if(isBleMeshSiteOnline()){                      //如果蓝牙站点在线，则走蓝牙站点控制
                LOG_PURPLE << "ble_site is online....";
                lightControlBleSite(area);     //关闭指定房间的灯，或关闭所有房间的灯
                response.set_content(okResponse.dump(), "text/json");
                return 0;
            }
        }


        //单个设备控制：zigbee灯，空调，净化器，加湿器....
        qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();         //获取所有列表
        qlibc::QData controlList = DownCommandData(requestData).getContorlData(deviceList); //得到控制列表
        Json::ArrayIndex controlListSize = controlList.size();

        std::map<string, qlibc::QData> deviceListMap;   //依据来源分类的设备列表
        for(Json::ArrayIndex i = 0; i < controlListSize; ++i){
            //按来源归类设备控制命令
            qlibc::QData controlData = controlList.getArrayElement(i);
            auto pos = deviceListMap.find(controlData.getString("sourceSite"));
            if(pos != deviceListMap.end()){
                pos->second.append(controlData);
            }else{
                deviceListMap.insert(std::make_pair(controlData.getString("sourceSite"), qlibc::QData().append(controlData)));
            }
        }

        //发送控制命令到相应的站点
        for(auto& elem : deviceListMap){
            qlibc::QData controlRequest, controlResponse;
            controlRequest.setString("service_id", "control_device");
            controlRequest.putData("request", qlibc::QData().putData("device_list", elem.second));
            LOG_GREEN << elem.first << ": controlRequest: " << controlRequest.toJsonString();
            SiteRecord::getInstance()->sendRequest2Site(elem.first, controlRequest, controlResponse);
            LOG_BLUE << elem.first << ": controlResponse: " << controlResponse.toJsonString();
        }

        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }


    int bleDeviceOperation_service_handler(const Request& request, Response& response){
        LOG_INFO << qlibc::QData(request.body).toJsonString(true);
        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }

    int updateDeviceList_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->updateDeviceList_service_handler...";
        DeviceManager::getInstance()->updateDeviceList();
        qlibc::QData ret;
        ret.setInt("code", 0);
        ret.setString("msg", "success");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }

    int updateGroupList_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->updateGroupList_service_handler...";
        GroupManager::getInstance()->updateGroupList();
        qlibc::QData ret;
        ret.setInt("code", 0);
        ret.setString("msg", "success");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }


    int getPanelList_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->getPanelList_service_handler: " << qlibc::QData(request.body).toJsonString();
        qlibc::QData panelList = siteManager::getPanelList();
        qlibc::QData data;
        data.setInt("code", 0);
        data.setString("error", "ok");
        data.putData("response", qlibc::QData().putData("panelList", panelList));
        response.set_content(data.toJsonString(), "text/json");
        return 0;
    }

    int getDeviceSiteList_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->getDeviceSiteList_service_handler: " << qlibc::QData(request.body).toJsonString();
        std::set<string> siteNames = SiteRecord::getInstance()->getSiteName();
        qlibc::QData siteArray;
        for(auto& siteName : siteNames){
            siteArray.append(siteName);
        }
        qlibc::QData data;
        data.setInt("code", 0);
        data.setString("error", "ok");
        data.putData("response", qlibc::QData().putData("siteArray", siteArray));
        response.set_content(data.toJsonString(), "text/json");
        return 0;
    }

    void sendRequest(const Request& request, Response& response){
        qlibc::QData requestData(request.body), responseData;
        string sourceSite = requestData.getData("request").getString("sourceSite");
        requestData.asValue()["request"].removeMember("sourceSite");
        std::set<string> siteNames = SiteRecord::getInstance()->getSiteName();
        for(auto& siteName : siteNames){
            if(siteName == sourceSite){
                LOG_GREEN << "requestData: " << requestData.toJsonString();
                SiteRecord::getInstance()->sendRequest2Site(siteName, requestData, responseData);
                LOG_BLUE << "responseData: " << responseData.toJsonString();
                if(responseData.empty()){
                    qlibc::QData ret;
                    ret.setInt("code", -1);
                    ret.setString("msg", "response is empty");
                    ret.putData("response", qlibc::QData());
                    response.set_content(ret.toJsonString(), "text/json");
                }else{
                    response.set_content(responseData.toJsonString(), "text/json");
                }
                return;
            }
        }

        //没有找到对应的站点
        qlibc::QData ret;
        ret.setInt("code", -1);
        ret.setString("msg", "need sourceSite to deliever!!!");
        ret.putData("response", qlibc::QData());
        response.set_content(ret.toJsonString(), "text/json");
    }


//重置网关
    int reset_device_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->reset_device_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//扫描设备
    int scan_device_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->scan_device_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//添加设备
    int add_device_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->add_device_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//添加zigbee设备
    int addZigbee_device_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->addZigbee_device_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//删除设备
    int del_device_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->del_device_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//配置设备属性
    int configProperty_device_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->configProperty_device_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//配置灯带属性
    int configNightStrip_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->configNightStrip_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//控制灯带灯珠
    int stripPointControl_service_handler(const Request& request, Response& response){
        qlibc::QData requestData(request.body);
        LOG_INFO << "synergy->stripPointControl_service_handler: " << requestData.toJsonString();
        sendRequest(request, response);
        return 0;
    }

//设备控制
    int deviceControl_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->deviceControl_service_handler: " << qlibc::QData(request.body).toJsonString();
        std::map<string, qlibc::QData> controlDeviceListMap;
        qlibc::QData controlDeviceList = qlibc::QData(request.body).getData("request").getData("device_list");
        Json::ArrayIndex size = controlDeviceList.size();
        for(Json::ArrayIndex i = 0; i < size; ++i){
            qlibc::QData item = controlDeviceList.getArrayElement(i);
            string device_id = item.getString("device_id");
            string inSourceSite = item.getString("sourceSite");
            string outSourceSite;
            if(DeviceManager::getInstance()->isInDeviceList(device_id, inSourceSite, outSourceSite)){    //待控制设备存在于设备列表中
                //按设备来源归类设备
                auto pos = controlDeviceListMap.find(outSourceSite);
                if(pos != controlDeviceListMap.end()){
                    pos->second.append(DeviceManager::getInstance()->restoreMac(item, inSourceSite));
                }else{
                    controlDeviceListMap.insert(
                            std::make_pair(outSourceSite,qlibc::QData().append(DeviceManager::getInstance()->restoreMac(item, inSourceSite))));
                }
            }
        }

        //发送控制命令到相应的站点
        for(auto& elem : controlDeviceListMap){    //按设备来源发送命令
            qlibc::QData controlRequest, controlResponse;
            controlRequest.setString("service_id", "control_device");
            controlRequest.putData("request", qlibc::QData().putData("device_list", elem.second));
            LOG_GREEN << elem.first << " controlRequest: " << controlRequest.toJsonString();
            SiteRecord::getInstance()->sendRequest2Site(elem.first, controlRequest, controlResponse);
            LOG_BLUE << elem.first << " controlResponse: " << controlResponse.toJsonString();
        }

        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }

//获取设备列表：ble, zigbee, tvAdapter
    int getDeviceList_service_handler(const Request &request, Response &response) {
        LOG_INFO << "synergy->getDeviceList_service_handler";
        qlibc::QData res;
        res.putData("device_list", DeviceManager::getInstance()->getAllDeviceList());

        qlibc::QData data;
        data.setInt("code", 0);
        data.setString("error", "ok");
        data.putData("response", res);

        response.set_content(data.toJsonString(), "text/json");
        return 0;
    }

//获取设备状态
    int get_device_state_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->get_device_state_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//创建分组
    int create_group_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->create_group_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//删除分组，删除前先将设备移除分组
    int delete_group_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->delete_group_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//重命名分组
    int rename_group_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->rename_group_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//添加设备进入分组
    int addDevice2Group_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->addDevice2Group_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//从分组移除设备
    int removeDeviceFromGroup_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->removeDeviceFromGroup_service_handler: " << qlibc::QData(request.body).toJsonString();
        sendRequest(request, response);
        return 0;
    }

//组控制
    int groupControl_service_handler(const Request& request, Response& response){
        LOG_INFO << "synergy->groupControl_service_handler: " << qlibc::QData(request.body).toJsonString();
        //判断设备属于哪个站点
        std::map<string, qlibc::QData> controlGroupListMap;
        qlibc::QData controlGroupList = qlibc::QData(request.body).getData("request").getData("group_list");
        Json::ArrayIndex size = controlGroupList.size();
        for(Json::ArrayIndex i = 0; i < size; ++i){
            qlibc::QData item = controlGroupList.getArrayElement(i);
            string group_id = item.getString("group_id");
            string inSourceSite = item.getString("sourceSite");
            string outSourceSite;
            if(GroupManager::getInstance()->isInGroupList(group_id, inSourceSite,outSourceSite)){
                auto pos = controlGroupListMap.find(outSourceSite);
                if(pos != controlGroupListMap.end()){
                    pos->second.append(GroupManager::getInstance()->restoreGrp(item, inSourceSite));
                }else{
                    controlGroupListMap.insert(std::make_pair(outSourceSite,
                                                              qlibc::QData().append(GroupManager::getInstance()->restoreGrp(item, inSourceSite))));
                }
            }
        }

        //发送控制命令到相应的站点
        for(auto& elem : controlGroupListMap){
            qlibc::QData controlRequest, controlResponse;
            controlRequest.setString("service_id", "control_group");
            controlRequest.putData("request", qlibc::QData().putData("group_list", elem.second));
            LOG_GREEN << elem.first << " controlRequest: " << controlRequest.toJsonString();
            SiteRecord::getInstance()->sendRequest2Site(elem.first, controlRequest, controlResponse);
            LOG_BLUE << elem.first << " controlResponse: " << controlResponse.toJsonString();
        }

        //如果group_id含有FFFF
        for(Json::ArrayIndex i = 0; i < size; ++i){
            qlibc::QData item = controlGroupList.getArrayElement(i);
            string group_id = item.getString("group_id");
            string sourceSite = item.getString("sourceSite");
            string group_uid = group_id;
            if(!sourceSite.empty()){
                group_uid.append(">").append(sourceSite);
            }

            smatch sm;
            if(regex_match(group_uid, sm,regex("FFFF>(.*)"))){
                item.setString("group_id", "FFFF");
                item.removeMember("sourceSite");
                string siteName = sm.str(1);

                qlibc::QData controlRequest, controlResponse;
                qlibc::QData groupList;
                groupList.append(item);
                controlRequest.setString("service_id", "control_group");
                controlRequest.putData("request", qlibc::QData().putData("group_list", groupList));
                LOG_GREEN << siteName << " controlRequest: " << controlRequest.toJsonString();
                SiteRecord::getInstance()->sendRequest2Site(siteName, controlRequest, controlResponse);
                LOG_BLUE << siteName << " controlResponse: " << controlResponse.toJsonString();
            }
        }

        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }

//获取分组列表：ble, zigbee
    int getGroupList_service_handler(const Request &request, Response &response) {
        LOG_INFO << "synergy->getGroupList_service_handler: " << qlibc::QData(request.body).toJsonString();
        LOG_INFO << "getGroupList_service_handler";
        qlibc::QData res;
        res.putData("group_list", GroupManager::getInstance()->getAllGroupList());

        qlibc::QData data;
        data.setInt("code", 0);
        data.setString("error", "ok");
        data.putData("response", res);

        response.set_content(data.toJsonString(), "text/json");
        return 0;
    }

    int voiceControl_service_handler(const Request& request, Response& response){
        LOG_INFO << "voiceControl_service_handler: " << qlibc::QData(request.body).toJsonString();
        qlibc::QData requestData(request.body);
        qlibc::QData controlData = requestData.getData("request");
        voiceMatch voiceCtrl(controlData);
        try{
            voiceCtrl.parseAndControl();
        }catch(const exception& e){
            LOG_RED << "exception: " << e.what();
            response.set_content(errResponse.dump(), "text/json");
            return 0;
        }
        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }

    void messagePublish(const Request& request){
        qlibc::QData requestData(request.body);
        LOG_INFO << "synergy->messagePublish: " << qlibc::QData(request.body).toJsonString();
        string message_id = requestData.getString("message_id");
        ServiceSiteManager::getInstance()->publishMessage(message_id, requestData.toJsonString());
        return;
    }

}