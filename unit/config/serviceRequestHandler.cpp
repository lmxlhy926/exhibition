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
        if(!payload.empty()){
            configParamUtil::getInstance()->saveWhiteListData(payload);
        }
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


void fileSync(string site_id, string getServiceId, string saveServiceId, string message){
    LOG_PURPLE << "==>start to fileSync: " << message;
    std::map<unsigned long long, Json::Value> resultMap;
    qlibc::QData node_list;
    Json::ArrayIndex node_list_size  = 0;

    //获取所有面板列表
    qlibc::QData request, response;
    request.setString("service_id", "get_all");
    request.putData("request", qlibc::QData());
    LOG_GREEN << "get node_list: " << request.toJsonString();
    bool siteBool = httpUtil::sitePostRequest("127.0.0.1", 9012, request, response);

    if(siteBool){
        node_list.setInitData(response.getData("response").getData("node_list"));
        node_list_size = node_list.size();
        LOG_BLUE << "node_list: " << node_list.toJsonString();

        for(Json::ArrayIndex i = 0; i < node_list_size; ++i){
            qlibc::QData item = node_list.getArrayElement(i);
            string ip = item.getString("ip");
            int port = 9006;
            qlibc::QData contentRequest, contentResponse;
            contentRequest.setString("service_id", getServiceId);
            contentRequest.putData("request", qlibc::QData());
            LOG_GREEN << "contentRequest: " << contentRequest.toJsonString();
            if(httpUtil::sitePostRequest(ip, port, contentRequest, contentResponse)){
                LOG_BLUE << "contentResponse";
                qlibc::QData content(contentResponse.getData("response"));
                if(!content.empty()){
                    unsigned long long timeStamp = 0;
                    try{
                        timeStamp = stoull(content.getString("timeStamp"), nullptr, 10);
                    }catch(const std::exception& e){
                        LOG_RED << "error in parse timeStamp....";
                        timeStamp = -1;
                    }
                    if(timeStamp > 0){
                        resultMap.insert(std::make_pair(timeStamp, content.asValue()));
                    }
                }else{
                    LOG_RED << "===>content is empty.....";
                }
            }
        }
    }

    if(resultMap.size() == 0){
        LOG_RED << "==>sync end, dont find any effective data.....";
        return;
    }

    //获得最新的内容
    auto pos = max_element(resultMap.begin(), resultMap.end());
    qlibc::QData newestContent(pos->second);

    //向所有站点更新内容
    for(Json::ArrayIndex i = 0; i < node_list_size; ++i){
        qlibc::QData item = node_list.getArrayElement(i);
        string ip = item.getString("ip");
        int port = 9006;
        qlibc::QData contentSaveRequest, contentSaveResponse;
        contentSaveRequest.setString("service_id", saveServiceId);
        contentSaveRequest.putData("request", newestContent);
        httpUtil::sitePostRequest(ip, port, contentSaveRequest, contentSaveResponse);
    }
    LOG_PURPLE << "==>sync completely.....";
}

int whiteList_get_service_request_handler(const Request& request, Response& response){
    LOG_GREEN << "===>whiteList_service_request_handler: " << qlibc::QData(request.body).toJsonString();
    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    LOG_GREEN << "get whiteList....";
    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", payload);

    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

int whiteList_sync_save_service_request_handler(const Request& request, Response& response){
    LOG_PURPLE << "===>recevied sync whiteList, start to compare...";
    qlibc::QData localWhiteList = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData whiteListData = qlibc::QData(request.body).getData("request");
    if(localWhiteList.empty() || whiteListData.empty()){
        LOG_RED << "===>localWhiteList or whiteListData is empty, dont save this data";
        return 0;
    }
    unsigned long long localTime, otherTime;
    try{
        localTime = stoull(localWhiteList.getString("timeStamp"), nullptr, 10);
        otherTime = stoull(whiteListData.getString("timeStamp"), nullptr, 10);
    }catch(const std::exception& e){
        LOG_RED << "==>error in transfering whiteListTime, dont save this data.......";
        return 0;
    }
    LOG_PURPLE << "==>whiteListlocalTime:" << localTime << ", whiteListotherTime: " << otherTime;

    if(localTime < otherTime){
        configParamUtil::getInstance()->saveWhiteListData(whiteListData);
        LOG_PURPLE << "===>save whiteList to the newest versiion....";
        //发布白名单修改信息
        qlibc::QData publishData;
        publishData.setString("message_id", WHITELIST_MODIFIED_MESSAGE_ID);
        publishData.putData("content", qlibc::QData());
        ServiceSiteManager::getInstance()->publishMessage(WHITELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
    }else{
        LOG_PURPLE << "===>whiteList is the newest, noting needed to save....";
    }

    return 0;
}

int whiteList_save_service_request_handler(const Request& request, Response& response){
    //收到白名单后，先将其保存
    qlibc::QData data(request.body);
    LOG_PURPLE << "==>whiteList_save_service_request_handler: " << data.toJsonString();
    qlibc::QData whiteListData = data.getData("request");
    if(whiteListData.empty()){
        LOG_RED << "==>white list is empty, format error....";
        qlibc::QData ret;
        ret.setInt("code", -1);
        ret.setString("msg", "white list is empty, format error....");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }

    configParamUtil::getInstance()->saveWhiteListData(whiteListData);

    //同步其它站点的白名单
    fileSync(CONFIG_SITE_ID, WHITELIST_REQUEST_SERVICE_ID, WHITELIST_SYNC_SAVE_REQUEST_SERVICE_ID,
             "whiteListSaveHandler invoke....");    //同步白名单

    //发布白名单修改信息
    qlibc::QData publishData;
    publishData.setString("message_id", WHITELIST_MODIFIED_MESSAGE_ID);
    publishData.putData("content", qlibc::QData());
    ServiceSiteManager::getInstance()->publishMessage(WHITELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());

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

int saveSceneFile_sync_service_request_handler(const Request& request, Response& response){
    qlibc::QData otherData = qlibc::QData(request.body).getData("request");
    qlibc::QData localData = configParamUtil::getInstance()->getSceneConfigFile();
    if(localData.empty() || otherData.empty()){
        LOG_RED << "===>localSceneData or otherSceneData is empty, dont save this data...";
        return 0;
    }
    unsigned long long localTime, otherTime;
    try{
        localTime = stoull(localData.getString("timeStamp"), nullptr, 10);
        otherTime = stoull(otherData.getString("timeStamp"), nullptr, 10);
    }catch(const std::exception& e){
        LOG_RED << "==>error in transfering sceneFileTime.......";
        return 0;
    }
    LOG_PURPLE << "==>sceneLocalTime:" << localTime << ", sceneOtherTime: " << otherTime;

    if(localTime < otherTime){
        LOG_PURPLE << "===>update sceneFile to the newest versiion....";
        configParamUtil::getInstance()->saveSceneConfigFile(otherData);
        //发布场景文件被修改消息
        qlibc::QData publishData;
        publishData.setString("message_id", SCENELIST_MODIFIED_MESSAGE_ID);
        publishData.putData("content", qlibc::QData());
        ServiceSiteManager::getInstance()->publishMessage(SCENELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
    }else{
        LOG_PURPLE << "===>sceneFile is the newest, noting needed to change....";
    }

    return 0;
}

//保存场景配置文件
int saveSceneFile_service_request_handler(const Request& request, Response& response){
    qlibc::QData data(request.body);
    LOG_INFO << "==>saveConfigFile_service_request_handler: " << data.toJsonString();
    qlibc::QData seceConfigFile = data.getData("request");
    configParamUtil::getInstance()->saveSceneConfigFile(seceConfigFile);

    //同步场景文件
    fileSync(CONFIG_SITE_ID, GET_SCENECONFIG_FILE_REQUEST_SERVICE_ID, SAVE_SYNC_SCENECONFIGFILE_REQUEST_SERVICE_ID,
             "sceneConfigFile invoke...");    //同步场景文件

    //发布场景文件被修改消息
    qlibc::QData publishData;
    publishData.setString("message_id", SCENELIST_MODIFIED_MESSAGE_ID);
    publishData.putData("content", qlibc::QData());
    ServiceSiteManager::getInstance()->publishMessage(SCENELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());

    qlibc::QData ret;
    ret.setInt("code", 0);
    ret.setString("msg", "success");
    response.set_content(ret.toJsonString(), "text/json");
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

//获取面板信息
int getPanelInfo_service_request_handler(const Request& request, Response& response){
    LOG_INFO << "==>getPanelInfo_service_request_handler: " << qlibc::QData(request.body).toJsonString();
    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", configParamUtil::getInstance()->getPanelInfo());
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

//设置面板信息
int setPanelInfo_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestData(request.body);
    LOG_INFO << "==>setPanelInfo_service_request_handler: " << requestData.toJsonString();
    qlibc::QData panelConfigData = requestData.getData("request");
    configParamUtil::getInstance()->setPanelInfo(panelConfigData);

    //发布面板配置信息改变
    qlibc::QData publishData;
    publishData.setString("message_id", PANELINFO_MODIFIED_MESSAGE_ID);
    publishData.putData("content", panelConfigData);
    ServiceSiteManager::getInstance()->publishMessage(PANELINFO_MODIFIED_MESSAGE_ID, publishData.toJsonString());
    LOG_INFO << "publish: " << publishData.toJsonString();

#if 0
    //将面板信息加入白名单中
    string origin_device_mac = panelConfigData.getString("device_mac");
    string device_mac;
    regex sep("[:]+");
    sregex_token_iterator end;
    sregex_token_iterator p(origin_device_mac.cbegin(), origin_device_mac.cend(), sep, {-1});
    for(; p != end; p++){
        device_mac += p->str();
    }

    //构造设备项
    qlibc::QData panelItem;
    panelItem.setString("category_code", "panel");
    panelItem.setString("device_sn", device_mac);
    panelItem.setString("device_mac", device_mac);
    panelItem.setString("device_name", panelConfigData.getString("device_name"));
    panelItem.setString("room_name", panelConfigData.getData("location").getString("room_name"));
    panelItem.setString("room_no", panelConfigData.getData("location").getString("room_no"));
    panelItem.setString("device_type", "2");

    //获取白名单
    qlibc::QData whiteList = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData devices = whiteList.getData("info").getData("devices");
    Json::ArrayIndex deviceSize = devices.size();
    int elem2DelIndex = -1;
    for(Json::ArrayIndex i = 0; i < deviceSize; ++i){
        qlibc::QData item = devices.getArrayElement(i);
        if(item.getString("device_mac") == device_mac){
            elem2DelIndex = i;
            break;
        }
    }

    //删除原先的项
    if(elem2DelIndex != -1){
        devices.deleteArrayItem(elem2DelIndex);
    }
    devices.append(panelItem);

    //保存白名单
    whiteList.asValue()["info"]["devices"] = devices.asValue();
    whiteList.setString("timeStamp", std::to_string(time(nullptr)));
    qlibc::QData contentSaveRequest, contentSaveResponse;
    contentSaveRequest.setString("service_id", WHITELIST_SAVE_REQUEST_SERVICE_ID);
    contentSaveRequest.putData("request", whiteList);
    httpUtil::sitePostRequest("127.0.0.1", 9006, contentSaveRequest, contentSaveResponse);
#endif

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", qlibc::QData());
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

int getAudioPanelList_service_request_handler(const Request& request, Response& response){
    LOG_INFO << "getAudioPanelList_service_request_handler: " << qlibc::QData(request.body).toJsonString();
    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData deviceList = payload.getData("info").getData("devices");
    qlibc::QData rooms = payload.getData("info").getData("rooms");

    qlibc::QData devices;
    for(Json::ArrayIndex i = 0; i < deviceList.size(); ++i){
        qlibc::QData item = deviceList.getArrayElement(i);
        if(item.getString("category_code") == "audiopanel"){
            devices.append(item);
        }
    }

    qlibc::QData retData;
    retData.putData("devices", devices);
    retData.putData("rooms", rooms);

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", retData);
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

//数组转换为map
std::map<string, Json::Value> array2Map(qlibc::QData& array, string key){
    std::map<string, Json::Value> map;
    Json::ArrayIndex arrySize = array.size();
    for(Json::ArrayIndex i = 0; i < arrySize; ++i){
        qlibc::QData item = array.getArrayElement(i);
        map.insert(std::make_pair(item.getString(key), item.asValue()));
    }
    return map;
}

void removePanelList(qlibc::QData& setDevices){
    std::map<string, Json::Value> setDevicesMap = array2Map(setDevices, "device_sn");
    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData originDevices  = payload.getData("info").getData("devices");
    std::map<string, Json::Value> originDevicesMap = array2Map(originDevices, "device_sn");

    //语音面板设备处理
    for(auto pos = originDevicesMap.begin(); pos != originDevicesMap.end();){
        if(pos->second["category_code"] == "audiopanel"){
            if(setDevicesMap.find(pos->first) == setDevicesMap.end()){
                pos = originDevicesMap.erase(pos);
            }else{
                ++pos;
            }
        }else{
            ++pos;
        }
    }

    //构造白名单
    Json::Value newDevices;
    for(auto& elem : originDevicesMap){
        newDevices.append(elem.second);
    }
    payload.asValue()["info"]["devices"] = newDevices;
    configParamUtil::getInstance()->saveWhiteListData(payload);
}


int saveAudioPanelList_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestData(request.body);
    LOG_INFO << "saveAudioPanelList_service_request_handler: " << requestData.toJsonString();
    qlibc::QData devices = requestData.getData("request").getData("devices");
    qlibc::QData rooms = requestData.getData("request").getData("rooms");

    //从白名单中删除不存在的设备
    removePanelList(devices);

    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    for(Json::ArrayIndex i = 0; i < devices.size(); ++i){
        qlibc::QData item = devices.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex = 0;

        qlibc::QData whiteListDevices(payload.getData("info").getData("devices"));
        for(Json::ArrayIndex j = 0; j < whiteListDevices.size(); ++j){
            qlibc::QData originItem = whiteListDevices.getArrayElement(j);
            if(item.getString("device_sn") == originItem.getString("device_sn")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }

        if(!hasItem){
            payload.asValue()["info"]["devices"].append(devices.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["devices"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["devices"].append(devices.getArrayElement(i).asValue());
        }

        qlibc::QData panelConfigData = configParamUtil::getInstance()->changePanelProperty(devices.getArrayElement(i));
        if(!panelConfigData.empty()){
            qlibc::QData publishData;
            publishData.setString("message_id", PANELINFO_MODIFIED_MESSAGE_ID);
            publishData.putData("content", panelConfigData);
            ServiceSiteManager::getInstance()->publishMessage(PANELINFO_MODIFIED_MESSAGE_ID, publishData.toJsonString());
            LOG_INFO << "publish: " << publishData.toJsonString();
        }
    }

    for(Json::ArrayIndex i = 0; i < rooms.size(); ++i){
        qlibc::QData item = rooms.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex{0};

        qlibc::QData whiteListRooms(payload.getData("info").getData("rooms"));
        for(Json::ArrayIndex j = 0; j < whiteListRooms.size(); ++j){
            qlibc::QData originItem = whiteListRooms.getArrayElement(j);
            if(item.getString("roomNo") == originItem.getString("roomNo")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["rooms"].append(rooms.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["rooms"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["rooms"].append(rooms.getArrayElement(i).asValue());
        }
    }
    payload.setString("timeStamp", requestData.getData("request").getString("timeStamp"));

    //保存设备列表
    qlibc::QData contentSaveRequest, contentSaveResponse;
    contentSaveRequest.setString("service_id", WHITELIST_SAVE_REQUEST_SERVICE_ID);
    contentSaveRequest.putData("request", payload);
    httpUtil::sitePostRequest("127.0.0.1", 9006, contentSaveRequest, contentSaveResponse);

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", qlibc::QData());
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}


//删除原有白名单中不存在的属性信息
void removeNonExist(qlibc::QData& setDevices, qlibc::QData& setDoors, qlibc::QData& setRooms, qlibc::QData& set_area_app){
    std::map<string, Json::Value> setDevicesMap = array2Map(setDevices, "device_sn");
    std::map<string, Json::Value> setDoorsMap   = array2Map(setDoors, "id");
    std::map<string, Json::Value> setAreaAppMap = array2Map(set_area_app, "area_id");

    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData originDevices  = payload.getData("info").getData("devices");
    qlibc::QData originDoors    = payload.getData("info").getData("doors");
    qlibc::QData originAreaApp  = payload.getData("info").getData("area_app");

    std::map<string, Json::Value> originDevicesMap = array2Map(originDevices, "device_sn");
    std::map<string, Json::Value> originDoorsMap   = array2Map(originDoors, "id");
    std::map<string, Json::Value> originAreaAppMap = array2Map(originAreaApp, "area_id");

    //雷达设备处理
    for(auto pos = originDevicesMap.begin(); pos != originDevicesMap.end();){
        if(pos->second["category_code"] == "radar"){
            if(setDevicesMap.find(pos->first) == setDevicesMap.end()){
               pos = originDevicesMap.erase(pos);
            }else{
                ++pos;
            }
        }else{
            ++pos;
        }
    }

    //门处理
    for(auto pos = originDoorsMap.begin(); pos != originDoorsMap.end();){
        if(setDoorsMap.find(pos->first) == setDoorsMap.end()){
            pos = originDoorsMap.erase(pos);
        }else{
            ++pos;
        }
    }

    //区域处理
    for(auto pos = originAreaAppMap.begin(); pos != originAreaAppMap.end();){
        if(setAreaAppMap.find(pos->first) == setAreaAppMap.end()){
            pos = originAreaAppMap.erase(pos);
        }else{
            ++pos;
        }
    }

    //构造白名单
    Json::Value newDevices, newDoors, newRooms, newAreaApps;
    for(auto& elem : originDevicesMap){
        newDevices.append(elem.second);
    }
    for(auto& elem : originDoorsMap){
        newDoors.append(elem.second);
    }
    for(auto& elem : originAreaAppMap){
        newAreaApps.append(elem.second);
    }
    payload.asValue()["info"]["devices"] = newDevices;
    payload.asValue()["info"]["doors"] = newDoors;
    payload.asValue()["info"]["area_app"] = newAreaApps;
    configParamUtil::getInstance()->saveWhiteListData(payload);
}


/*
 * 无则增加
 * 有则修改
 * 不存在则删除
 */
int setRadarDevice_service_request_handler(const Request& request, Response& response){
    qlibc::QData requestData(request.body);
    LOG_INFO << "setRadarDevice_service_request_handler: " << requestData.toJsonString();
    qlibc::QData devices = requestData.getData("request").getData("devices");
    qlibc::QData doors = requestData.getData("request").getData("doors");
    qlibc::QData rooms = requestData.getData("request").getData("rooms");
    qlibc::QData area_app = requestData.getData("request").getData("area_app");

    //从白名单中删除不存在的雷达设备、门、区域
    removeNonExist(devices, doors, rooms, area_app);

    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();

    for(Json::ArrayIndex i = 0; i < devices.size(); ++i){
        qlibc::QData item = devices.getArrayElement(i);
        bool hasItem{false};
        unsigned deleteIndex = 0;
        qlibc::QData whiteListDevices(payload.getData("info").getData("devices"));
        for(Json::ArrayIndex j = 0; j < whiteListDevices.size(); ++j){
            qlibc::QData originItem = whiteListDevices.getArrayElement(j);
            if(item.getString("device_sn") == originItem.getString("device_sn")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["devices"].append(devices.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["devices"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["devices"].append(devices.getArrayElement(i).asValue());
        }
    }

    for(Json::ArrayIndex i = 0; i < doors.size(); ++i){
        qlibc::QData item = doors.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex{0};
        qlibc::QData whiteListDoors(payload.getData("info").getData("doors"));
        for(Json::ArrayIndex j = 0; j < whiteListDoors.size(); ++j){
            qlibc::QData originItem = whiteListDoors.getArrayElement(j);
            if(item.getString("id") == originItem.getString("id")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["doors"].append(doors.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["doors"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["doors"].append(doors.getArrayElement(i).asValue());
        }
    }

    for(Json::ArrayIndex i = 0; i < rooms.size(); ++i){
        qlibc::QData item = rooms.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex{0};
        qlibc::QData whiteListRooms(payload.getData("info").getData("rooms"));
        for(Json::ArrayIndex j = 0; j < whiteListRooms.size(); ++j){
            qlibc::QData originItem = whiteListRooms.getArrayElement(j);
            if(item.getString("roomNo") == originItem.getString("roomNo")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["rooms"].append(rooms.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["rooms"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["rooms"].append(rooms.getArrayElement(i).asValue());
        }
    }

    for(Json::ArrayIndex i = 0; i < area_app.size(); ++i){
        qlibc::QData item = area_app.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex{0};
        qlibc::QData whiteListAreaApp(payload.getData("info").getData("area_app"));
        for(Json::ArrayIndex j = 0; j < whiteListAreaApp.size(); ++j){
            qlibc::QData originItem = whiteListAreaApp.getArrayElement(j);
            if(item.getString("area_id") == originItem.getString("area_id")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["area_app"].append(area_app.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["area_app"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["area_app"].append(area_app.getArrayElement(i).asValue());
        }
    }
    payload.setString("timeStamp", std::to_string(time(nullptr)));

    //保存设备列表
    qlibc::QData contentSaveRequest, contentSaveResponse;
    contentSaveRequest.setString("service_id", WHITELIST_SAVE_REQUEST_SERVICE_ID);
    contentSaveRequest.putData("request", payload);
    httpUtil::sitePostRequest("127.0.0.1", 9006, contentSaveRequest, contentSaveResponse);

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", qlibc::QData());
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}

void receiveRadarDevice_message_handler(const Request& request){
    qlibc::QData requestData(request.body);
    LOG_INFO << "receiveRadarDevice_message_handler: " << requestData.toJsonString();
    qlibc::QData devices = requestData.getData("content").getData("devices");
    qlibc::QData doors = requestData.getData("content").getData("doors");
    qlibc::QData rooms = requestData.getData("content").getData("rooms");
    qlibc::QData area_app = requestData.getData("content").getData("area_app");

    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();

    for(Json::ArrayIndex i = 0; i < devices.size(); ++i){
        qlibc::QData item = devices.getArrayElement(i);
        bool hasItem{false};
        unsigned deleteIndex = 0;
        qlibc::QData whiteListDevices(payload.getData("info").getData("devices"));
        for(Json::ArrayIndex j = 0; j < whiteListDevices.size(); ++j){
            qlibc::QData originItem = whiteListDevices.getArrayElement(j);
            if(item.getString("device_sn") == originItem.getString("device_sn")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["devices"].append(devices.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["devices"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["devices"].append(devices.getArrayElement(i).asValue());
        }
    }

    for(Json::ArrayIndex i = 0; i < doors.size(); ++i){
        qlibc::QData item = doors.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex{0};
        qlibc::QData whiteListDoors(payload.getData("info").getData("doors"));
        for(Json::ArrayIndex j = 0; j < whiteListDoors.size(); ++j){
            qlibc::QData originItem = whiteListDoors.getArrayElement(j);
            if(item.getString("id") == originItem.getString("id")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["doors"].append(doors.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["doors"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["doors"].append(doors.getArrayElement(i).asValue());
        }
    }

    for(Json::ArrayIndex i = 0; i < rooms.size(); ++i){
        qlibc::QData item = rooms.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex{0};
        qlibc::QData whiteListRooms(payload.getData("info").getData("rooms"));
        for(Json::ArrayIndex j = 0; j < whiteListRooms.size(); ++j){
            qlibc::QData originItem = whiteListRooms.getArrayElement(j);
            if(item.getString("roomNo") == originItem.getString("roomNo")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["rooms"].append(rooms.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["rooms"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["rooms"].append(rooms.getArrayElement(i).asValue());
        }
    }

    for(Json::ArrayIndex i = 0; i < area_app.size(); ++i){
        qlibc::QData item = area_app.getArrayElement(i);
        bool hasItem{false};
        unsigned int deleteIndex{0};
        qlibc::QData whiteListAreaApp(payload.getData("info").getData("area_app"));
        for(Json::ArrayIndex j = 0; j < whiteListAreaApp.size(); ++j){
            qlibc::QData originItem = whiteListAreaApp.getArrayElement(j);
            if(item.getString("area_id") == originItem.getString("area_id")){
                hasItem = true;
                deleteIndex = j;
                break;
            }
        }
        if(!hasItem){
            payload.asValue()["info"]["area_app"].append(area_app.getArrayElement(i).asValue());
        }else{
            Json::Value removeValue;
            payload.asValue()["info"]["area_app"].removeIndex(deleteIndex, &removeValue);
            payload.asValue()["info"]["area_app"].append(area_app.getArrayElement(i).asValue());
        }
    }
    payload.setString("timeStamp", std::to_string(time(nullptr)));

    //保存设备列表
    qlibc::QData contentSaveRequest, contentSaveResponse;
    contentSaveRequest.setString("service_id", WHITELIST_SAVE_REQUEST_SERVICE_ID);
    contentSaveRequest.putData("request", payload);
    httpUtil::sitePostRequest("127.0.0.1", 9006, contentSaveRequest, contentSaveResponse);
}




#if 0
int whiteList_update_service_request_handler(const Request& request, Response& response){
    LOG_HLIGHT << "==>whiteList_update_service_request_handler";
    //蓝牙站点所有设备
    qlibc::QData bleSiteDeviceList = qlibc::QData(request.body).getData("request").getData("device_list");
    size_t bleSiteDeviceListSize = bleSiteDeviceList.size();

    //原有列表
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
#endif