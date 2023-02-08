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
#include "../deviceGroupManage/deviceManager.h"
#include "../deviceGroupManage/groupManager.h"
#include "../control/sceneCommand.h"
#include "../voiceControl/voiceStringMatch.h"
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


    void classify(qlibc::QData &controlData, qlibc::QData &bleDeviceList, qlibc::QData &zigbeeDeviceList,
                  qlibc::QData &tvAdapterList) {
        string sourceSite = controlData.getString("sourceSite");
        if (sourceSite == BleSiteID) {
            bleDeviceList.append(controlData);
        } else if (sourceSite == ZigbeeSiteID) {
            zigbeeDeviceList.append(controlData);
        } else if (sourceSite == TvAdapterSiteID) {
            tvAdapterList.append(controlData);
        }
    }


    bool sendCmd(qlibc::QData &bleDeviceList, qlibc::QData &zigbeeDeviceList, qlibc::QData &tvAdapterList) {
        qlibc::QData controlData, response;
        controlData.setString("service_id", "control_device");
        if (zigbeeDeviceList.size() > 0) {
            qlibc::QData list;
            list.putData("device_list", zigbeeDeviceList);
            controlData.putData("request", list);
            SiteRecord::getInstance()->sendRequest2Site(ZigbeeSiteID, controlData, response);
            LOG_YELLOW << "cmd2ZigbeeSite: " << controlData.toJsonString();
        }
        if (tvAdapterList.size() > 0) {
            qlibc::QData list;
            list.putData("device_list", tvAdapterList);
            controlData.putData("request", list);
            SiteRecord::getInstance()->sendRequest2Site(TvAdapterSiteID, controlData, response);
            LOG_YELLOW << "cmd2TvadapterSite: " << controlData.toJsonString();
        }
        return true;
    }

    //构造分组控制指令
    qlibc::QData buildControlCmd(string group_id){
        qlibc::QData command, commandList, deviceItem, deviceList;
        command.setString("command_id", "luminance_color_temperature");
        command.setInt("command_para_luminance", 0);
        command.setInt("command_para_color_temperature", 6500);
        command.setInt("transTime", 0);
        commandList.append(command);

        deviceItem.setString("group_id", group_id);
        deviceItem.putData("command_list", commandList);

        qlibc::QData requestData;
        requestData.setString("service_id", "control_group");
        requestData.putData("request", qlibc::QData().putData("group_list", qlibc::QData().append(deviceItem)));

        return requestData;
    }

    bool isSiteOnline(const std::string& siteId){
        qlibc::QData requestData, responseData;
        requestData.setString("service_id", "get_message_list");
        requestData.putData("request", qlibc::QData());
        return httpUtil::sitePostRequest("127.0.0.1", 9001, requestData, responseData);
    }

    //软服针对样板间下发给mesh站点的控制指令
    void lightControlBleSite(const string& area, const qlibc::QData& requestData){
        if(area == "all"){      //关闭所有灯
            qlibc::QData controlData = buildControlCmd("FFFF");
            LOG_YELLOW << controlData.toJsonString();
            qlibc::QData resData;
            std::set<string> siteName = SiteRecord::getInstance()->getSiteName();
            for(auto& elem : siteName){
                smatch sm;
                if(regex_match(elem, sm, regex("(.*):(.*)"))){
                    if(sm.str(2) == BleSiteID){
                        SiteRecord::getInstance()->sendRequest2Site(elem, controlData, resData);
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
            string room_no = item.getData("location").getString("room_no");
            if(item.getBool("areaFullGroup") && area == room_no){
                //构造灯控指令
                string group_id = item.getString("group_id");
                qlibc::QData controlData = buildControlCmd(group_id);
                LOG_YELLOW << controlData.toJsonString();
                qlibc::QData resData;
                std::set<string> siteName = SiteRecord::getInstance()->getSiteName();
                for(auto& elem : siteName){
                    smatch sm;
                    if(regex_match(elem, sm, regex("(.*):(.*)"))){
                        if(sm.str(2) == BleSiteID){
                            SiteRecord::getInstance()->sendRequest2Site(elem, controlData, resData);
                        }
                    }
                }
                break;
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
        for(auto& elem : sceneVec){     //通过action判断是否是场景指令
            if(action == elem){     //场景指令
                //发布场景指令消息
                qlibc::QData publishData;
                publishData.setString("message_id", Scene_Msg_MessageID);
                publishData.putData("content", requestData.getData("request"));
                ServiceSiteManager::getInstance()->publishMessage(Scene_Msg_MessageID, publishData.toJsonString());

                //进行格式转换，请求场景站点接口
                qlibc::QData siteResponse;
                SceneCommand sc(requestData);
                bool flag = sc.sendCmd(siteResponse);
                if(!flag){
                    response.set_content(errResponse.dump(), "text/json");
                }else{
                    LOG_HLIGHT << "==>sitResponse: " << siteResponse.toJsonString();
                    response.set_content(siteResponse.toJsonString(), "text/json");
                }
                return 0;
            }
        }

        //针对样板间的mesh灯
        if(code == "light"){
            if(isSiteOnline(BleSiteID)){    //如果蓝牙站点在线，则走蓝牙站点控制
                lightControlBleSite(area, requestData);
                return 0;
            }
        }

        //针对tvAdapter下挂的多媒体设备
        qlibc::QData bleDeviceList, zigbeeDeviceList, tvAdapterList;
        qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
        qlibc::QData controlList = DownCommandData(requestData).getContorlData(deviceList);
        Json::ArrayIndex controlListSize = controlList.size();
        std::map<string, qlibc::QData> deviceListMap;
        for(Json::ArrayIndex i = 0; i < controlListSize; ++i){
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
            SiteRecord::getInstance()->sendRequest2Site(elem.first, controlRequest, controlResponse);
        }

        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }

    int getDeviceList_service_handler(const Request &request, Response &response) {
        LOG_INFO << "getDeviceList_service_handler";
        qlibc::QData res;
        res.putData("device_list", DeviceManager::getInstance()->getAllDeviceList());

        qlibc::QData data;
        data.setInt("code", 0);
        data.setString("error", "ok");
        data.putData("response", res);

        response.set_content(data.toJsonString(), "text/json");
        return 0;
    }

    int getGroupList_service_handler(const Request &request, Response &response) {
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
        string voiceControlString = requestData.getData("request").getString("controlString");
        voiceStringMatchControl voiceCtrl(voiceControlString);
        voiceCtrl.parseAndControl();
        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }

    int deviceControl_service_handler(const Request& request, Response& response){
        LOG_INFO << "deviceControl_service_handler: " << qlibc::QData(request.body).toJsonString();
        //判断设备属于哪个站点
        std::map<string, qlibc::QData> deviceListMap;
        qlibc::QData deviceList = qlibc::QData(request.body).getData("request").getData("device_list");
        Json::ArrayIndex size = deviceList.size();
        for(Json::ArrayIndex i = 0; i < size; ++i){
            qlibc::QData item = deviceList.getArrayElement(i);
            string device_id = item.getString("device_id");
            string sourceSite;
            if(DeviceManager::getInstance()->isInDeviceList(device_id, sourceSite)){
                auto pos = deviceListMap.find(sourceSite);
                if(pos != deviceListMap.end()){
                    pos->second.append(item);
                }else{
                    deviceListMap.insert(std::make_pair(sourceSite, qlibc::QData().append(item)));
                }
            }
        }

        //发送控制命令到相应的站点
        for(auto& elem : deviceListMap){
            qlibc::QData controlRequest, controlResponse;
            controlRequest.setString("service_id", "control_device");
            controlRequest.putData("request", qlibc::QData().putData("device_list", elem.second));
            LOG_GREEN << elem.first << "controlRequest: " << controlRequest.toJsonString();
            SiteRecord::getInstance()->sendRequest2Site(elem.first, controlRequest, controlResponse);
            LOG_BLUE << elem.first << " controlResponse: " << controlResponse.toJsonString();
        }

        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }


    int groupControl_service_handler(const Request& request, Response& response){
        LOG_INFO << "groupControl_service_handler: " << qlibc::QData(request.body).toJsonString();
        //判断设备属于哪个站点
        std::map<string, qlibc::QData> groupListMap;
        qlibc::QData groupList = qlibc::QData(request.body).getData("request").getData("group_list");
        Json::ArrayIndex size = groupList.size();
        for(Json::ArrayIndex i = 0; i < size; ++i){
            qlibc::QData item = groupList.getArrayElement(i);
            string group_id = item.getString("group_id");
            string sourceSite;
            if(GroupManager::getInstance()->isInGroupList(group_id, sourceSite)){
                auto pos = groupListMap.find(sourceSite);
                if(pos != groupListMap.end()){
                    pos->second.append(item);
                }else{
                    groupListMap.insert(std::make_pair(sourceSite, qlibc::QData().append(item)));
                }
            }
        }

        //发送控制命令到相应的站点
        for(auto& elem : groupListMap){
            qlibc::QData controlRequest, controlResponse;
            controlRequest.setString("service_id", "control_group");
            controlRequest.putData("request", qlibc::QData().putData("group_list", elem.second));
            LOG_GREEN << elem.first << "controlRequest: " << controlRequest.toJsonString();
            SiteRecord::getInstance()->sendRequest2Site(elem.first, controlRequest, controlResponse);
            LOG_BLUE << elem.first << " controlResponse: " << controlResponse.toJsonString();
        }

        response.set_content(okResponse.dump(), "text/json");
        return 0;
    }
}