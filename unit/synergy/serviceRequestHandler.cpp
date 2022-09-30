//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"
#include "siteService/nlohmann/json.hpp"
#include "qlibc/QData.h"
#include "param.h"
#include "common/httpUtil.h"
#include "deviceControl/common.h"
#include "log/Logging.h"
#include "deviceManager.h"
#include "groupManager.h"

namespace synergy {

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

        if (bleDeviceList.size() > 0) {
            qlibc::QData list;
            list.putData("device_list", bleDeviceList);
            controlData.putData("request", list);
            SiteRecord::getInstance()->sendRequest2Site(BleSiteID, controlData, response);
        }
        if (zigbeeDeviceList.size() > 0) {
            qlibc::QData list;
            list.putData("device_list", zigbeeDeviceList);
            controlData.putData("request", zigbeeDeviceList);
            SiteRecord::getInstance()->sendRequest2Site(ZigbeeSiteID, controlData, response);
        }
        if (tvAdapterList.size() > 0) {
            qlibc::QData list;
            list.putData("device_list", tvAdapterList);
            controlData.putData("request", tvAdapterList);
            SiteRecord::getInstance()->sendRequest2Site(TvAdapterSiteID, controlData, response);
        }
        return true;
    }


    bool controlDeviceRightNow(qlibc::QData &message) {
        std::cout << "received message: " << message.toJsonString() << std::endl;
        qlibc::QData deviceList = DeviceManager::getInstance()->getAllDeviceList();
        qlibc::QData controlData = DownCommandData(message).getContorlData(deviceList);

        qlibc::QData bleDeviceList, zigbeeDeviceList, tvAdapterList;
        classify(controlData, bleDeviceList, zigbeeDeviceList, tvAdapterList);
        sendCmd(bleDeviceList, zigbeeDeviceList, tvAdapterList);

        return true;
    }

    int device_control_service_handler(const Request &request, Response &response) {
        qlibc::QData requestBody(request.body);
        LOG_INFO << "device_control_service_handler: " << requestBody.toJsonString();
        if (requestBody.type() != Json::nullValue) {
            controlDeviceRightNow(requestBody);
            response.set_content(okResponse.dump(), "text/json");
        } else {
            response.set_content(errResponse.dump(), "text/json");
        }
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

    int sceneCommand_service_handler(const Request &request, Response &response) {
        qlibc::QData requestData(request.body);
        LOG_INFO << "sceneCommand_service_handler: " << requestData.toJsonString();
        qlibc::QData requestBody = requestData.getData("request");

        string type = requestBody.getString("type");
        string action = requestBody.getString("action");
        string code = requestBody.getString("code");
        string delay = requestBody.getString("delay");
        qlibc::QData inParams = requestBody.getData("inParams");

        qlibc::QData req, siteRequest, siteResponse;
        if (action == "constantBrightnessModeStart") {  //开启恒定照度
            req.setBool("active_switch", true);
            req.setInt("target_illumination", inParams.getInt("brightness"));
            req.setInt("target_temperature", inParams.getInt("colourTemperature"));
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));

            siteRequest.setString("service_id", "constant_illuminance");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        } else if (action == "constantBrightnessModeFlag") {    //设置恒定照度
            string flag = inParams.getString("flag");
            bool active_switch;
            if (flag == "0") {
                active_switch = true;
            } else {
                active_switch = false;
            }

            req.setBool("active_switch", active_switch);
            req.setInt("target_illumination", 0);
            req.setInt("target_temperature", 0);
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));

            siteRequest.setString("service_id", "constant_illuminance");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        }else if(action == "comfortableDinnerModeStart"){   //温馨就餐开启
            req.setBool("active_switch", true);
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
            siteRequest.setString("service_id", "dining");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        }else if(action == "comfortableDinnerModeStop"){    //温馨就餐关闭
            req.setBool("active_switch", false);
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
            siteRequest.setString("service_id", "dining");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        }else if(action == "readModeStart"){    //阅读模式开启
            req.setBool("active_switch", true);
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
            siteRequest.setString("service_id", "reading");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        }else if(action == "cookModeStart"){    //烹饪模式开启
            req.setBool("active_switch", true);
            req.setInt("target_temperature", inParams.getInt("colourTemperature"));
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
            siteRequest.setString("service_id", "cooking");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        }else if(action == "enterHouseholdModeStart"){  //回家模式客厅有人
            req.setBool("active_switch", true);
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
            siteRequest.setString("service_id", "enter_home");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        }else if(action == "enterHouseholdNoPersonModeStart"){  //回家模式无人
            req.setBool("active_switch", false);
            req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
            siteRequest.setString("service_id", "enter_home");
            siteRequest.putData("request", req);
            SiteRecord::getInstance()->sendRequest2Site(SceneSiteID, siteRequest, siteResponse);
            LOG_YELLOW << "cmd: " << siteRequest.toJsonString();

        } else {
            LOG_RED << action << " is not supported.....";
            response.set_content(errResponse.dump(), "text/json");
            return 0;
        }

        LOG_HLIGHT << "==>sitResponse: " << siteResponse.toJsonString();
        response.set_content(siteResponse.toJsonString(), "text/json");
        return 0;
    }

}