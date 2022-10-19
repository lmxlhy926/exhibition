//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "param.h"
#include "common/httpUtil.h"
#include "control/downCommand.h"
#include "log/Logging.h"
#include "deviceManager.h"
#include "groupManager.h"
#include "control/sceneCommand.h"
#include <vector>

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

      "viewingSceneStart"
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


    int cloudCommand_service_handler(const Request& request, Response& response){
        qlibc::QData requestData(request.body);
        LOG_INFO << "cloudCommand_service_handler: " << requestData.toJsonString();
        string action = requestData.getData("request").getString("action");

        //场景指令
        for(auto& elem : sceneVec){
            if(action == elem){     //场景指令
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

        //第三方设备控制命令
        qlibc::QData bleDeviceList, zigbeeDeviceList, tvAdapterList;
        qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
        qlibc::QData controlData = DownCommandData(requestData).getContorlData(deviceList);
        classify(controlData, bleDeviceList, zigbeeDeviceList, tvAdapterList);
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