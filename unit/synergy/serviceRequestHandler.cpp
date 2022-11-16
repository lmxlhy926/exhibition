//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "qlibc/QData.h"
#include "param.h"
#include "common/httpUtil.h"
#include "control/downCommand.h"
#include "log/Logging.h"
#include "deviceManager.h"
#include "groupManager.h"
#include "control/sceneCommand.h"
#include <vector>

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

    qlibc::QData buildControlCmd(string device_id){
        qlibc::QData command, commandList, deviceItem, deviceList;
        command.setString("command_id", "luminance_color_temperature");
        command.setInt("command_para_luminance", 0);
        command.setInt("command_para_color_temperature", 6500);
        command.setInt("transTime", 0);
        commandList.append(command);

        deviceItem.setString("group_id", device_id);
        deviceItem.putData("command_list", commandList);

        qlibc::QData requestData;
        requestData.setString("service_id", "control_group");
        requestData.putData("request", qlibc::QData().putData("group_list", qlibc::QData().append(deviceItem)));

        return requestData;
    }

    bool isSiteOnline(const std::string& siteId){
        qlibc::QData data, responseData;
        data.setString("service_id", "get_message_list");
        data.putData("request", qlibc::QData());
        return SiteRecord::getInstance()->sendRequest2Site(siteId, data, responseData);
    }

    //使用蓝牙站点
    void lightControlBleSite(const string& area, const qlibc::QData& requestData){
        if(area == "all"){      //关闭所有灯
            qlibc::QData controlData = buildControlCmd("FFFF");
            LOG_YELLOW << controlData.toJsonString();
            qlibc::QData resData;
            SiteRecord::getInstance()->sendRequest2Site(BleSiteID, controlData, resData);
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
                string commandValue = requestData.getData("request").getData("inParams").getString("power");
                qlibc::QData controlData = buildControlCmd(group_id);
                LOG_YELLOW << controlData.toJsonString();
                qlibc::QData resData;
                SiteRecord::getInstance()->sendRequest2Site(BleSiteID, controlData, resData);
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

        //场景指令
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

        //使用蓝牙站点
        if(code == "light"){
            if(isSiteOnline(BleSiteID)){
                lightControlBleSite(area, requestData);
                return 0;
            }
        }

        //否则，则为第三方设备控制命令
        qlibc::QData bleDeviceList, zigbeeDeviceList, tvAdapterList;
        qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
        qlibc::QData controlList = DownCommandData(requestData).getContorlData(deviceList);
        Json::ArrayIndex controlListSize = controlList.size();
        for(Json::ArrayIndex i = 0; i < controlListSize; ++i){
            qlibc::QData controlData = controlList.getArrayElement(i);
            classify(controlData, bleDeviceList, zigbeeDeviceList, tvAdapterList);
        }
        sendCmd(bleDeviceList, zigbeeDeviceList, tvAdapterList);

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
}