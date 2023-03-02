//
// Created by 78472 on 2022/5/11.
//

#include "serviceRequestHandler.h"
#include "param.h"
#include "common/httpUtil.h"
#include "siteService/service_site_manager.h"
#include "util/mqttPayloadHandle.h"
#include "log/Logging.h"
#include <algorithm>
using namespace servicesite;

int test_service_request_handler(const Request& request, Response& response) {
    // HTTP库已判断字符串能否转成 JSON

    // 请求的json字符串位于request.body
    auto request_json = json::parse(request.body);

    printf("request:\n%s\n", request_json.dump(4).c_str());

    // 服务反馈
    json response_json = {
            {"code", 0},
            {"error", "ok"},
            {"response", {
                             {"test_data", "from test_service_id_1"}
                     }}
    };
    response.set_content(response_json.dump(), "text/plain");


    return 0;
}

void extractFromWhiteList(qlibc::QData& deviceList){
    qlibc::QData whiteList = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData whiteListDevices = whiteList.getData("info").getData("devices");
    size_t size = whiteListDevices.size();
    for(size_t i = 0; i < size; ++i){
        qlibc::QData ithData;
        whiteListDevices.getArrayElement(i, ithData);
        qlibc::QData data;
        data.setString("device_id", ithData.getString("device_sn"));
        data.setString("online_state", "online");

        deviceList.append(data);
    }
}


int sceneListRequest_service_request_handler(const Request& request, Response& response, bool isConnec) {
    LOG_INFO << "===>sceneListRequest_service_request_handler: " << request.body;
    if (!isConnec){
        LOG_RED << "===>cant access internet......";
        qlibc::QData errData;
        errData.setInt("code", 1);
        errData.setString("error", "cant access internet");
        errData.putData("response", qlibc::QData());
        response.set_content(errData.toJsonString(), "text/json");
        return 0;
    }

    qlibc::QData sceneListRequest, sceneListResponse;
    qlibc::QData param;
    param.setString("familyCode", "did:chisid:0x88508ea0601591e2afc95b662a9b279e75ef3f95");  //TODO 待定
    sceneListRequest.putData("param", param);
    sceneListRequest.setString("User-Agent", "curl");

    cloudUtil::getInstance()->ecb_httppost(SCENELIST_URL, sceneListRequest, sceneListResponse);

    qlibc::QData data;
    if(sceneListResponse.getInt("code") == 200){
        data.setInt("code", 0);
    }else
        data.setInt("code", 1);

    data.setString("error", sceneListResponse.getString("msg"));
    data.putData("response", sceneListResponse.getData("data"));

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

int subDeviceRegister_service_request_handler(const Request& request, Response& response, bool isConnec) {
    LOG_INFO << "===>subDeviceRegister_service_request_handler: " << request.body;
    if (!isConnec){
        LOG_RED << "===>cant access internet......";
        qlibc::QData errData;
        errData.setInt("code", 1);
        errData.setString("error", "cant access internet");
        errData.putData("response", qlibc::QData());
        response.set_content(errData.toJsonString(), "text/json");
        return 0;
    }

    qlibc::QData requestData(request.body);
    qlibc::QData registerRequest, registerResponse;
    qlibc::QData param = requestData.getData("request");
    param.setString("domainID", configParamUtil::getInstance()->getBaseInfo().getString("domainID"));
    registerRequest.setString("User-Agent", "curl");
    registerRequest.putData("param", param);

    cloudUtil::getInstance()->ecb_httppost(SUBDEVICE_REGISTER_URL, registerRequest, registerResponse);

    qlibc::QData data;
    if(registerResponse.getInt("code") == 200){
        data.setInt("code", 0);
    }else{
        data.setInt("code", 1);
    }
    data.setString("error", registerResponse.getString("msg"));
    data.putData("response", qlibc::QData());
    response.set_content(data.toJsonString(), "text/json");

    return 0;
}


int postDeviceList_service_request_handler(const Request& request, Response& response, bool isConnec){
    LOG_INFO << "===>postDeviceList_service_request_handler: " << request.body;
    if (!isConnec){
        LOG_RED << "===>cant access internet......";
        qlibc::QData errData;
        errData.setInt("code", 1);
        errData.setString("error", "cant access internet");
        errData.putData("response", qlibc::QData());
        response.set_content(errData.toJsonString(), "text/json");
        return 0;
    }

    qlibc::QData requestData(request.body);
    qlibc::QData registerRequest, registerResponse;
    qlibc::QData deviceList = requestData.getData("request").getData("deviceList");
    qlibc::QData param;
    param.setString("domainID", configParamUtil::getInstance()->getBaseInfo().getString("domainID"));
    param.putData("smartDeviceEdgeDTOS", deviceList);
    registerRequest.setString("User-Agent", "curl");
    registerRequest.putData("param", param);

    cloudUtil::getInstance()->ecb_httppost(POSTDEVICELIST_URL, registerRequest, registerResponse);

    qlibc::QData data;
    if(registerResponse.getInt("code") == 200){
        data.setInt("code", 0);
    }else{
        data.setInt("code", 1);
    }
    data.setString("error", registerResponse.getString("msg"));
    data.putData("response", qlibc::QData());
    response.set_content(data.toJsonString(), "text/json");

    return 0;
}

int domainIdRequest_service_request_handler(const Request& request, Response& response, bool isConnec) {
    LOG_INFO << "===>domainIdRequest_service_request_handler: " << request.body;

    string domainId = configParamUtil::getInstance()->getBaseInfo().getString("domainID");
    qlibc::QData res;
    res.setString("domainId", domainId);

    qlibc::QData data;
    if(domainId.empty()){
        data.setInt("code", 1);
        data.setString("error", "domainID为空");
    }else{
        data.setInt("code", 0);
        data.setString("error", "ok");
    }
    data.putData("response", res);
    response.set_content(data.toJsonString(), "text/json");

    return 0;
}

int engineer_service_request_handler(mqttClient& mc, const Request& request, Response& response) {
    LOG_INFO << "===>engineer_service_request_handler: " << request.body;
    if (!mc.isConnected()){
        LOG_RED << "===>cant access internet......";
        qlibc::QData errData;
        errData.setInt("code", 1);
        errData.setString("error", "cant access internet");
        errData.putData("response", qlibc::QData());
        response.set_content(errData.toJsonString(), "text/json");
        return 0;
    }

    qlibc::QData requestData = qlibc::QData(request.body).getData("request");
    qlibc::QData registerRes;
    cloudUtil::getInstance()->tvRegister(mc, requestData, registerRes);

    qlibc::QData data;
    if(registerRes.getInt("code") == 200){
        data.setInt("code", 0);
    }else{
        data.setInt("code", 1);
    }
    data.setString("error", registerRes.getString("msg"));
    data.putData("response", qlibc::QData());

    response.set_content(data.toJsonString(), "text/json");

    return 0;
}


int getWhiteListFromCloud_service_request_handler(mqttClient& mc, const Request& request, Response& response){
    LOG_INFO << "===>getWhiteListFromCloud_service_request_handler: " << request.body;
    if (!mc.isConnected()){
        LOG_RED << "===>cant access internet......";
        qlibc::QData errData;
        errData.setInt("code", 1);
        errData.setString("error", "cant access internet");
        errData.putData("response", qlibc::QData());
        response.set_content(errData.toJsonString(), "text/json");
        return 0;
    }

    string domainID = configParamUtil::getInstance()->getBaseInfo().getString("domainID");
    qlibc::QData whiteListRequest, whiteListResponse;
    qlibc::QData param;
    param.setString("familyCode", domainID);
    whiteListRequest.putData("param", param);
    whiteListRequest.setString("User-Agent", "curl");

    cloudUtil::getInstance()->ecb_httppost(WHITELIST_REQUEST_URL, whiteListRequest, whiteListResponse);

    qlibc::QData data;
    if(whiteListResponse.getInt("code") == 200){
        string payloadString = whiteListResponse.getData("data").toJsonString();
        qlibc::QData payload = mqttPayloadHandle::transform(payloadString.c_str(), payloadString.size());
        configParamUtil::getInstance()->saveWhiteListData(payload);

        data.setInt("code", 0);
        data.setString("error", whiteListResponse.getString("msg"));
        data.putData("response", payload);

    }else{
        data.setInt("code", 1);
        data.setString("error", whiteListResponse.getString("msg"));
        data.putData("response", qlibc::QData());
    }

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

void whiteList_sync(string site_id, string getServiceId, string saveServiceId){
    LOG_PURPLE << "start to sync " << site_id << " ......";
    std::map<int, Json::Value> resultMap;
    qlibc::QData app_site_list;
    Json::ArrayIndex app_site_list_size  = 0;

    //获取所有面板上的保存内容
    qlibc::QData request, response;
    request.setString("service_id", "get_app_site_list");
    request.putData("request", qlibc::QData());
    if(httpUtil::sitePostRequest("127.0.0.1", 9012, request, response)){
        app_site_list.setInitData(response.getData("response").getData("app_site_list"));
        app_site_list_size = app_site_list.size();
        for(Json::ArrayIndex i = 0; i < app_site_list_size; ++i){
            qlibc::QData item = app_site_list.getArrayElement(i);
            if(item.getString("site_id") == site_id){
                qlibc::QData contentRequest, contentResponse;
                contentRequest.setString("service_id", getServiceId);
                contentRequest.putData("request", qlibc::QData());
                if(httpUtil::sitePostRequest(item.getString("ip"), 9006, contentRequest, contentResponse)){
                    qlibc::QData content(contentResponse.getData("response"));
                    if(!content.empty()){
                        resultMap.insert(std::make_pair(atoi(content.getString("timeStamp").c_str()), content.asValue()));
                    }
                }
            }
        }
    }

    //获得最新的内容
    auto pos = max_element(resultMap.begin(), resultMap.end());
    qlibc::QData newestContent(pos->second);

    //向所有站点更新内容
    for(Json::ArrayIndex i = 0; i < app_site_list_size; ++i){
        qlibc::QData item = app_site_list.getArrayElement(i);
        if(item.getString("site_id") == site_id){
            qlibc::QData contentSaveRequest, contentSaveResponse;
            contentSaveRequest.setString("service_id", saveServiceId);
            contentSaveRequest.putData("request", newestContent);
            httpUtil::sitePostRequest(item.getString("ip"), 9006, contentSaveRequest, contentSaveResponse);
        }
    }
}


int whiteList_service_request_handler(const Request& request, Response& response){
    LOG_INFO << "===>whiteList_service_request_handler: " << qlibc::QData(request.body).toJsonString();

    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", payload);

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

int whiteList_save_service_request_handler(const Request& request, Response& response){
    qlibc::QData data(request.body);
    LOG_INFO << "==>whiteList_save_service_request_handler: " << data.toJsonString();
    qlibc::QData whiteListData = data.getData("request");
    configParamUtil::getInstance()->saveWhiteListData(whiteListData);

    whiteList_sync(CONFIG_SITE_ID, WHITELIST_REQUEST_SERVICE_ID, WHITELIST_SYNC_SAVE_REQUEST_SERVICE_ID);    //同步白名单

    qlibc::QData ret;
    ret.setInt("code", 0);
    ret.setString("msg", "success");
    response.set_content(ret.toJsonString(), "text/json");

    return 0;
}

int whiteList_sync_save_service_request_handler(const Request& request, Response& response){
    qlibc::QData whiteListData = qlibc::QData(request.body).getData("request");
    qlibc::QData localWhiteList = configParamUtil::getInstance()->getWhiteList();
    if(atoi(localWhiteList.getString("timeStamp").c_str()) < atoi(whiteListData.getString("timeStamp").c_str())){
        configParamUtil::getInstance()->saveWhiteListData(whiteListData);
        //发布白名单修改信息
        qlibc::QData publishData;
        publishData.setString("message_id", WHITELIST_MODIFIED_MESSAGE_ID);
        publishData.putData("content", qlibc::QData());
        ServiceSiteManager::getInstance()->publishMessage(WHITELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
    }
    return 0;
}

int whiteList_update_service_request_handler(const Request& request, Response& response){
    LOG_HLIGHT << "==>whiteList_update_service_request_handler";
    qlibc::QData bleSiteDeviceList = qlibc::QData(request.body).getData("request").getData("device_list");
    size_t bleSiteDeviceListSize = bleSiteDeviceList.size();

    qlibc::QData configWhiteList = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData devices = configWhiteList.getData("info").getData("devices");

    for(Json::ArrayIndex i = 0; i < bleSiteDeviceListSize; ++i){
        qlibc::QData deviceItem = bleSiteDeviceList.getArrayElement(i);
        string device_id = deviceItem.getString("device_id");
        string device_type = deviceItem.getString("device_type");
        string device_typeCode = deviceItem.getString("device_typeCode");
        string device_modelCode = deviceItem.getString("device_modelCode");
        string room_name = deviceItem.getData("location").getString("room_name");
        string room_no = deviceItem.getData("location").getString("room_no");

        for(Json::ArrayIndex j = 0; j < devices.size(); ++j){
            if(devices.getArrayElement(j).getString("device_sn") == device_id){
                break;
            }else if(devices.getArrayElement(j).getString("device_sn") != device_id && j == devices.size() -1){
                qlibc::QData item;
                item.setString("category_code", device_type);
                item.setString("device_sn", device_id);
                item.setString("device_type", device_typeCode);
                item.setString("device_model", device_modelCode);
                item.setString("room_name", room_name);
                item.setString("room_no", room_no);
                devices.append(item);
                break;
            }
        }
        if(devices.size() == 0){
            qlibc::QData item;
            item.setString("category_code", device_type);
            item.setString("device_sn", device_id);
            item.setString("device_type", device_typeCode);
            item.setString("device_model", device_modelCode);
            item.setString("room_name", room_name);
            item.setString("room_no", room_no);
            devices.append(item);
        }
    }

    configWhiteList.asValue()["info"]["devices"] = devices.asValue();
    configParamUtil::getInstance()->saveWhiteListData(configWhiteList);

    qlibc::QData ret;
    ret.setInt("code", 0);
    ret.setString("msg", "success");
    response.set_content(ret.toJsonString(), "text/json");

    return 0;
}

int whiteList_delete_service_request_handler(const Request& request, Response& response){
    LOG_HLIGHT << "==>whiteList_delete_service_request_handler";
    qlibc::QData bleSiteDeviceList = qlibc::QData(request.body).getData("request").getData("device_list");
    size_t bleSiteDeviceListSize = bleSiteDeviceList.size();

    qlibc::QData configWhiteList = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData devices = configWhiteList.getData("info").getData("devices");

    for(Json::ArrayIndex i = 0; i < bleSiteDeviceListSize; ++i){
        qlibc::QData deviceItem = bleSiteDeviceList.getArrayElement(i);
        string device_id = deviceItem.getString("device_id");
        for(Json::ArrayIndex j = 0; j < devices.size(); ++j){
            if(devices.getArrayElement(j).getString("device_sn") == device_id){
                devices.deleteArrayItem(j);
            }
        }
    }

    configWhiteList.asValue()["info"]["devices"] = devices.asValue();
    configParamUtil::getInstance()->saveWhiteListData(configWhiteList);

    qlibc::QData ret;
    ret.setInt("code", 0);
    ret.setString("msg", "success");
    response.set_content(ret.toJsonString(), "text/json");

    return 0;
}


//获取场景配置文件
int getSceneFile_service_request_handler(const Request& request, Response& response){
    LOG_INFO << "===>getConfigFile_service_request_handler: " << qlibc::QData(request.body).toJsonString();

    qlibc::QData payload = configParamUtil::getInstance()->getSceneConfigFile();
    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", payload);

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

//保存场景配置文件
int saveSceneFile_service_request_handler(const Request& request, Response& response){
    qlibc::QData data(request.body);
    LOG_INFO << "==>saveConfigFile_service_request_handler: " << data.toJsonString();
    qlibc::QData seceConfigFile = data.getData("request");
    configParamUtil::getInstance()->saveSceneConfigFile(seceConfigFile);

    whiteList_sync(CONFIG_SITE_ID, GET_SCENECONFIG_FILE_REQUEST_SERVICE_ID, SAVE_SYNC_SCENECONFIGFILE_REQUEST_SERVICE_ID);    //同步场景文件

    qlibc::QData ret;
    ret.setInt("code", 0);
    ret.setString("msg", "success");
    response.set_content(ret.toJsonString(), "text/json");
    return 0;
}

int saveSceneFile_sync_service_request_handler(const Request& request, Response& response){
    qlibc::QData data = qlibc::QData(request.body).getData("request");
    qlibc::QData localData = configParamUtil::getInstance()->getSceneConfigFile();
    if(atoi(localData.getString("timeStamp").c_str()) < atoi(data.getString("timeStamp").c_str())){
        configParamUtil::getInstance()->saveSceneConfigFile(data);
        //发布场景文件被修改消息
        qlibc::QData publishData;
        publishData.setString("message_id", SCENELIST_MODIFIED_MESSAGE_ID);
        publishData.putData("content", qlibc::QData());
        ServiceSiteManager::getInstance()->publishMessage(SCENELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
    }

    return 0;
}

int getAllDeviceList_service_request_handler(const Request& request, Response& response){
    //请求tvAdapter设备列表, 请求雷达、语音面板设备列表
    LOG_INFO << "getAllDeviceList_service_request_handler" << request.body;

    //像tvAdapter、zigbee、南向站点获取设备列表
    qlibc::QData deviceListRequest;
    qlibc::QData adapterResponse, zigBeeResponse;
    deviceListRequest.setString("service_id", "get_device_list");
    deviceListRequest.putData("request", qlibc::QData());

    httpUtil::sitePostRequest(RequestIp, AdapterPort, deviceListRequest, adapterResponse);
    httpUtil::sitePostRequest(RequestIp, ZigBeeSitePort, deviceListRequest, zigBeeResponse);

    //初始列表为空
    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");

    //组装设备列表
    qlibc::QData deviceListAdapter = adapterResponse.getData("response").getData("device_list");
    qlibc::QData deviceListZigbee = zigBeeResponse.getData("response").getData("device_list");

    qlibc::QData deviceList;
    for(int i = 0; i < deviceListAdapter.size(); i++){
        qlibc::QData deviceItem = deviceListAdapter.getArrayElement(i);
        deviceItem.setString("souceSite", "tv_adapter");
        deviceList.append(deviceItem);
    }
    for(int i = 0; i < deviceListZigbee.size(); i++){
        qlibc::QData deviceItem = deviceListZigbee.getArrayElement(i);
        deviceItem.setString("souceSite", "zigbee_light");
        deviceList.append(deviceItem);
    }

    //加入白名单设备
    extractFromWhiteList(deviceList);

    //写入获取到的设备列表
    qlibc::QData receiveRes;
    receiveRes.putData("device_list", deviceList);
    data.putData("response", receiveRes);

    response.set_content(data.toJsonString(), "text/json");

    return 0;
}