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
#include "whiteListUtil.h"
using namespace servicesite;

static std::mutex audioRadarServiceMutex;
static std::mutex syncSaveMutex;

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

void subscribeFromAllConfigSite(std::vector<string>& messageIdList){
    //获取所有面板列表
    qlibc::QData request, response;
    request.setString("service_id", "get_all");
    request.putData("request", qlibc::QData());
    LOG_GREEN << "get node_list: " << request.toJsonString();
    bool siteBool = httpUtil::sitePostRequest("127.0.0.1", 9012, request, response);
    if(siteBool){
        qlibc::QData node_list;
        node_list.setInitData(response.getData("response").getData("node_list"));
        Json::ArrayIndex node_list_size = node_list.size();
        for(Json::ArrayIndex i = 0; i < node_list_size; ++i){
            qlibc::QData item = node_list.getArrayElement(i);
            string ip = item.getString("ip");
            int port = 9006;
            ServiceSiteManager::subscribeMessage(ip, port, messageIdList);
        }
    }
}


//找到本面板对应的账号手机号
void addPhone2PanelConfig(qlibc::QData& devices){
    Json::ArrayIndex deviceListSize = devices.size();
    string panelSn = configParamUtil::getInstance()->getPanelInfo().getString("device_mac");
    if(panelSn.empty() || deviceListSize == 0){
        return;
    }

    for(int i = 0; i < deviceListSize; ++i){
        qlibc::QData item = devices.getArrayElement(i);
        if(item.getString("device_sn") == panelSn){
            string phone = item.getString("phone");
            if(!phone.empty()){
                qlibc::QData panelConfigData = configParamUtil::getInstance()->changePanelProperty(item);
                qlibc::QData publishData;
                publishData.setString("message_id", PANELINFO_MODIFIED_MESSAGE_ID);
                publishData.putData("content", panelConfigData);
                ServiceSiteManager::getInstance()->publishMessage(PANELINFO_MODIFIED_MESSAGE_ID, publishData.toJsonString());
                LOG_INFO << "publish self_info_update message: " << publishData.toJsonString();
                break;
            }
        }
    }
}

void whiteListFileSync(string site_id, string getServiceId, string message){
    std::lock_guard<std::mutex> lg(syncSaveMutex);
    LOG_INFO << "***************************************************************";
    LOG_INFO << "***************************************************************";
    LOG_PURPLE << "==>whiteList fileSync start: " << message << "---------------";
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
                qlibc::QData content(contentResponse.getData("response"));
                if(!content.empty()){
                    unsigned long long timeStamp = 0;
                    try{
                        timeStamp = stoull(content.getString("timeStamp"), nullptr, 10);
                    }catch(const std::exception& e){
                        LOG_RED << "contentResponse has error in parse timeStamp....";
                        timeStamp = 0;
                    }
                    if(timeStamp > 0){
                        resultMap.insert(std::make_pair(timeStamp, content.asValue()));
                        LOG_BLUE << "contentResponse is inserted to map....";
                    }
                }else{
                    LOG_RED << "===>contentResponse is empty.....";
                }
            }else{
                LOG_RED << "contentRequest send filed....";
            }
        }
    }else{
        LOG_RED << "get node_list failed....." ;
    }

    if(resultMap.size() == 0){
        LOG_RED << "==>sync end, dont find any effective data-----------";
    }else if(resultMap.size() == 1 && !configParamUtil::getInstance()->getWhiteList().empty()){
        LOG_PURPLE << "==>only get ownWhiteList, no need to merge.....";
    }else{
        LOG_PURPLE << "==>start to merge whiteList.....";
        qlibc::QData finalWhiteList = getMergeWhiteList(resultMap);
        configParamUtil::getInstance()->saveWhiteListData(finalWhiteList);
        //更新本地面板panelConfig信息
        qlibc::QData devices = finalWhiteList.getData("info").getData("devices");
        addPhone2PanelConfig(devices);
        //发布白名单改变消息
        qlibc::QData publishData;
        publishData.setString("message_id", WHITELIST_MODIFIED_MESSAGE_ID);
        publishData.putData("content", finalWhiteList);     
        ServiceSiteManager::getInstance()->publishMessage(WHITELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
        LOG_PURPLE << "==>publish whiteList ....";
        LOG_PURPLE << "==>whiteList merge completely.....";
    }
    LOG_PURPLE << "==>whiteList fileSync end: " << message << "---------------";
    LOG_INFO << "***************************************************************";
    LOG_INFO << "***************************************************************";
}


void sceneFileSync(string site_id, string getServiceId, string message){
    std::lock_guard<std::mutex> lg(syncSaveMutex);
    LOG_INFO << "***************************************************************";
    LOG_INFO << "***************************************************************";
    LOG_PURPLE << "==>sceneFile fileSync start: " << message << "---------------";
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
                qlibc::QData content(contentResponse.getData("response"));
                if(!content.empty()){
                    unsigned long long timeStamp = 0;
                    try{
                        timeStamp = stoull(content.getString("timeStamp"), nullptr, 10);
                    }catch(const std::exception& e){
                        LOG_RED << "contentResponse has error in parse timeStamp....";
                        timeStamp = 0;
                    }
                    if(timeStamp > 0){
                        resultMap.insert(std::make_pair(timeStamp, content.asValue()));
                        LOG_BLUE << "contentResponse is inserted to map....";
                    }
                }else{
                    LOG_RED << "===>contentResponse is empty.....";
                }
            }else{
                LOG_RED << "contentRequest send filed....";
            }
        }
    }else{
        LOG_RED << "get node_list failed....." ;
    }

    if(resultMap.size() == 0){
        LOG_RED << "==>sync end, dont find any effective data-----------";
        return;
    }else if(resultMap.size() == 1 && !configParamUtil::getInstance()->getSceneConfigFile().empty()){
        LOG_PURPLE << "==>only get ownSceneFile, no need to update.....";
    }else{
        LOG_PURPLE << "==>sceneFile start to update.....";
        auto pos = max_element(resultMap.begin(), resultMap.end());
        qlibc::QData newestSceneFileData(pos->second);
        //保存最新的场景文件
        configParamUtil::getInstance()->saveSceneConfigFile(newestSceneFileData);
        //发布场景文件被修改消息
        qlibc::QData publishData;
        publishData.setString("message_id", SCENELIST_MODIFIED_MESSAGE_ID);
        publishData.putData("content", newestSceneFileData);
        ServiceSiteManager::getInstance()->publishMessage(SCENELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
        LOG_PURPLE << "publish sceneFile.....";
        LOG_PURPLE << "==>sceneFile update completely.....";
    }
    LOG_PURPLE << "==>sceneFile fileSync end: " << message << "---------------";
    LOG_INFO << "***************************************************************";
    LOG_INFO << "***************************************************************";
}

int whiteList_get_service_request_handler(const Request& request, Response& response){
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
    //收到白名单后，先将其保存
    qlibc::QData data(request.body);
    LOG_INFO << "==>whiteList_save_service_request_handler......";
    qlibc::QData whiteListData = data.getData("request");
    if(whiteListData.empty()){
        LOG_RED << "==>whiteList is empty, format error....";
        qlibc::QData ret;
        ret.setInt("code", -1);
        ret.setString("msg", "white list is empty, format error....");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }

    unsigned long long timeStamp = 0;
    try{
        timeStamp = stoull(whiteListData.getString("timeStamp"), nullptr, 10);
    }catch(const std::exception& e){
        LOG_RED << "receivedWhiteList has error in parse timeStamp, don't save this file....";
        qlibc::QData ret;
        ret.setInt("code", -1);
        ret.setString("msg", "error");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }

    //保存手机号到panelConfig文件中
    qlibc::QData devices = whiteListData.getData("info").getData("devices");
    addPhone2PanelConfig(devices);

    //将白名单存储到本机
    configParamUtil::getInstance()->saveWhiteListData(whiteListData);

    //发布白名单修改信息
    qlibc::QData publishData;
    publishData.setString("message_id", WHITELIST_MODIFIED_MESSAGE_ID);
    publishData.putData("content", configParamUtil::getInstance()->getWhiteList());     //发布最新的白名单
    ServiceSiteManager::getInstance()->publishMessage(WHITELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
    LOG_PURPLE << "==>publish whiteList......";

    //通知其它面板进行白名单合并
    qlibc::QData whiteListMerge;
    whiteListMerge.setString("message_id", WHITELIST_MERGE_MESSAGE_ID);
    whiteListMerge.putData("content", qlibc::QData());     //发布最新的白名单
    ServiceSiteManager::getInstance()->publishMessage(WHITELIST_MERGE_MESSAGE_ID, whiteListMerge.toJsonString());
    LOG_PURPLE << "==>publish whiteListMergeMessage: " << whiteListMerge.toJsonString();

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
    if(seceConfigFile.empty()){
        LOG_RED << "==>seceConfigFile is empty, format error....";
        qlibc::QData ret;
        ret.setInt("code", -1);
        ret.setString("msg", "seceConfigFile is empty, format error....");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }

    //判断时间戳
    unsigned long long timeStamp = 0;
    try{
        timeStamp = stoull(seceConfigFile.getString("timeStamp"), nullptr, 10);
    }catch(const std::exception& e){
        LOG_RED << "receivedScenefile has error in parse timeStamp, don't save this file....";
        qlibc::QData ret;
        ret.setInt("code", -1);
        ret.setString("msg", "error");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }

    //保存场景配置信息
    configParamUtil::getInstance()->saveSceneConfigFile(seceConfigFile);
    //发布场景文件被修改消息
    qlibc::QData publishData;
    publishData.setString("message_id", SCENELIST_MODIFIED_MESSAGE_ID);
    publishData.putData("content", seceConfigFile);
    ServiceSiteManager::getInstance()->publishMessage(SCENELIST_MODIFIED_MESSAGE_ID, publishData.toJsonString());
    LOG_INFO << "publish sceneFile......";

    //通知其它面板进行场景文件合并
    qlibc::QData sceneFileUpdateData;
    sceneFileUpdateData.setString("message_id", SCENEFILE_UPDATE_MESSAGE_ID);
    sceneFileUpdateData.putData("content", qlibc::QData());     //通知进行场景文件更新
    ServiceSiteManager::getInstance()->publishMessage(SCENEFILE_UPDATE_MESSAGE_ID, sceneFileUpdateData.toJsonString());
    LOG_PURPLE << "publish sceneFileUpdateMessage: " << sceneFileUpdateData.toJsonString();

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
    qlibc::QData area_app = payload.getData("info").getData("area_app");
    qlibc::QData doors = payload.getData("info").getData("doors");

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
    retData.putData("area_app", area_app);
    retData.putData("doors", doors);

    qlibc::QData data;
    data.setInt("code", 0);
    data.setString("error", "ok");
    data.putData("response", retData);
    response.set_content(data.toJsonString(), "text/json");
    return 0;
}


int saveAudioPanelList_service_request_handler(const Request& request, Response& response){
    std::lock_guard<std::mutex> lg(audioRadarServiceMutex);
    qlibc::QData requestData(request.body);
    LOG_INFO << "saveAudioPanelList_service_request_handler: " << requestData.toJsonString();
    string timeStamp = requestData.getData("request").getString("timeStamp");
    if(timeStamp.empty()){
        LOG_RED << "timeStamp is empty, failed to saveAudioPanelList....";
        qlibc::QData data;
        data.setInt("code", -1);
        data.setString("error", "ok");
        data.putData("response", qlibc::QData());
        response.set_content(data.toJsonString(), "text/json");
        return 0;
    }

    string phone = requestData.getData("request").getString("phone");
    qlibc::QData devices = requestData.getData("request").getData("devices");
    qlibc::QData rooms = requestData.getData("request").getData("rooms"); 

    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData localDevices = payload.getData("info").getData("devices");
    qlibc::QData localRooms = payload.getData("info").getData("rooms");

    payload.asValue()["info"]["devices"] = getHandledDeviceData(devices, localDevices).asValue();
    payload.asValue()["info"]["rooms"] = rooms.asValue();
    payload.setString("timeStamp", timeStamp);
    payload.setString("phone", phone);

    //保存白名单
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


int setRadarDevice_service_request_handler(const Request& request, Response& response){
    std::lock_guard<std::mutex> lg(audioRadarServiceMutex);
     qlibc::QData requestData(request.body);
    LOG_INFO << "setRadarDevice_service_request_handler: " << requestData.toJsonString();
    string timeStamp = requestData.getData("request").getString("timeStamp");
    if(timeStamp.empty()){
        LOG_RED << "timeStamp is empty, failed to setRadarDevice....";
        qlibc::QData data;
        data.setInt("code", -1);
        data.setString("error", "ok");
        data.putData("response", qlibc::QData());
        response.set_content(data.toJsonString(), "text/json");
        return 0;
    }

    string phone = requestData.getData("request").getString("phone");
    qlibc::QData devices = requestData.getData("request").getData("devices");
    qlibc::QData rooms = requestData.getData("request").getData("rooms");
    qlibc::QData doors = requestData.getData("request").getData("doors");
    qlibc::QData area_app = requestData.getData("request").getData("area_app");
    std::vector<string> radarVec = getRadarVec(devices, phone);

    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData localDevices = payload.getData("info").getData("devices");
    qlibc::QData localRooms = payload.getData("info").getData("rooms");
    qlibc::QData localDoors = payload.getData("info").getData("doors");
    qlibc::QData localAreas = payload.getData("info").getData("area_app");

    payload.asValue()["info"]["devices"] = getHandledDeviceData(devices, localDevices).asValue();
    payload.asValue()["info"]["doors"] = getHandledAreasDoorsDataAfterRadarService(devices, doors, localDoors, "radarsn", "id").asValue();
    payload.asValue()["info"]["area_app"] = getHandledAreasDoorsDataAfterRadarService(devices, area_app, localAreas, "radarsn", "area_id").asValue();
    payload.setString("timeStamp", timeStamp);
    payload.setString("phone", phone);

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


//订阅消息触发动作
void messageTrigger(const Request& request){
    qlibc::QData requestData(request.body);
    LOG_INFO << "messageTrigger: " << qlibc::QData(request.body).toJsonString();
    string message_id = requestData.getString("message_id");
    if(message_id == WHITELIST_MERGE_MESSAGE_ID){
        whiteListFileSync(CONFIG_SITE_ID, WHITELIST_REQUEST_SERVICE_ID, "messagetrigger update"); 
    }else if(message_id == SCENEFILE_UPDATE_MESSAGE_ID){
        sceneFileSync(CONFIG_SITE_ID, GET_SCENECONFIG_FILE_REQUEST_SERVICE_ID, "messagetrigger update");
    }
    return;
}


#if 0
qlibc::QData getDevicesAfterhandleRadarDevice(CategoryKeyMapType& devices, CategoryKeyMapType& localDevices, string& option){
    if(option == "update"){     //更新雷达设备
        for(auto pos = devices.begin(); pos != devices.end(); ++pos){
            //提取一条数据
            string phone = pos->first;
            std::map<string, Json::Value> radarDeviceMap = pos->second;

            //添加该条数据
            auto localPos = localDevices.find(phone);
            if(localPos != localDevices.end()){     //存在该账号
                std::map<string, Json::Value>& localRadarDeviceMap = localPos->second;
                for(auto& radarDevice : radarDeviceMap){
                    auto findPos = localRadarDeviceMap.find(radarDevice.first);
                    if(findPos != localRadarDeviceMap.end()){   //有则替换
                        localRadarDeviceMap.erase(findPos);
                        localRadarDeviceMap.insert(radarDevice);
                    }else{  //无则添加
                        localRadarDeviceMap.insert(radarDevice);
                    }
                }
            }else{
                localDevices.insert(*pos);
            }

            //删除重复设备
            for(auto position = localDevices.begin(); position != localDevices.end(); ++position){
                if(position->first != phone){
                    clearElementInReference(radarDeviceMap, position->second);
                }
            }
        }
    }else if(option == "delete"){   //删除雷达设备
         for(auto pos = devices.begin(); pos != devices.end(); ++pos){
            //提取一条数据
            string phone = pos->first;
            std::map<string, Json::Value> radarDeviceMap = pos->second;

            //在相同账号下搜索进行删除
            auto localPos = localDevices.find(phone);
            if(localPos != localDevices.end()){     //存在该账号
                std::map<string, Json::Value>& localRadarDeviceMap = localPos->second;
                for(auto& radarDevice : radarDeviceMap){
                    auto findPos = localRadarDeviceMap.find(radarDevice.first);
                    if(findPos != localRadarDeviceMap.end()){   //有则删除
                        localRadarDeviceMap.erase(findPos);
                    }
                }
            }

            //在不同账号下搜索进行删除
            for(auto position = localDevices.begin(); position != localDevices.end(); ++position){
                if(position->first != phone){
                    clearElementInReference(radarDeviceMap, position->second);
                }
            }
        }
    }

    return categoryKeyMap2JsonData(localDevices);
}


//添加删除雷达设备时，门和区域处理
qlibc::QData getDoorAreasAfterHandleRadarDevice(CategoryKeyMapType& radarSnKeyMap, CategoryKeyMapType& radarSnLocalKeyMap,
                                                const string& option){
    if(option == "update"){
        for(auto pos = radarSnKeyMap.begin(); pos != radarSnKeyMap.end(); ++pos){
            string radarSn = pos->first;
            auto position = radarSnLocalKeyMap.find(radarSn);
            if(position != radarSnLocalKeyMap.end()){
                position->second = pos->second;
            }else{
                radarSnLocalKeyMap.insert(std::make_pair(radarSn, pos->second));
            }
        }
    }else if(option == "delete"){
        std::vector<string> radarSnVec;
        for(auto radarDevice : radarSnKeyMap){
            radarSnVec.push_back(radarDevice.first);
        }

        for(auto& radarSn : radarSnVec){
            auto position = radarSnLocalKeyMap.find(radarSn);
            if(position != radarSnLocalKeyMap.end()){
                radarSnLocalKeyMap.erase(position);
            }
        }
    }

    return categoryKeyMap2JsonData(radarSnLocalKeyMap);
}


void radarMessageReceivedHandler(const Request& request){
    qlibc::QData receivedData(request.body);
    LOG_INFO << "radarMessageReceivedHandler: " << receivedData.toJsonString();
    //接收到的数据
    qlibc::QData devices = receivedData.getData("content").getData("info").getData("devices");
    qlibc::QData area_app = receivedData.getData("content").getData("info").getData("area_app");
    qlibc::QData doors = receivedData.getData("content").getData("info").getData("doors");
    qlibc::QData rooms = receivedData.getData("content").getData("info").getData("rooms");
    string option = receivedData.getData("content").getString("op");
    
    //本地数据
    qlibc::QData payload = configParamUtil::getInstance()->getWhiteList();
    qlibc::QData localDevices = payload.getData("info").getData("devices");
    qlibc::QData localAreas = payload.getData("info").getData("area_app");
    qlibc::QData localDoors = payload.getData("info").getData("doors");
    qlibc::QData localRooms = payload.getData("info").getData("rooms");
    string localTimeStampStr = payload.getString("timeStamp");
    unsigned long long localTime;
    try{
        localTime = stoull(localTimeStampStr, nullptr, 10);
    }catch(const std::exception& e){
        localTime = 1580572800;     //2020-02-02 00:00:00
    }
    
    //接收数据转换
    CategoryKeyMapType phoneRadarMap = JsonData2CategoryKeyMap(devices, "phone", "device_sn");
    CategoryKeyMapType areaRadarMap = JsonData2CategoryKeyMap(area_app, "radarsn", "area_id");
    CategoryKeyMapType doorsRadarMap = JsonData2CategoryKeyMap(doors, "radarsn", "id");

    //本地数据转换
    CategoryKeyMapType localPhoneRadarMap = JsonData2CategoryKeyMap(localDevices, "phone", "device_sn");
    CategoryKeyMapType localAreaRadarMap = JsonData2CategoryKeyMap(localAreas, "radarsn", "area_id");
    CategoryKeyMapType localDoorsRadarMap = JsonData2CategoryKeyMap(localDoors, "radarsn", "id");

    //获得最终数据
    qlibc::QData finalAreas = getDoorAreasAfterHandleRadarDevice(areaRadarMap, localAreaRadarMap, option);
    qlibc::QData finalDoors = getDoorAreasAfterHandleRadarDevice(doorsRadarMap, localDoorsRadarMap, option);
    qlibc::QData finalDevices = getDevicesAfterhandleRadarDevice(phoneRadarMap, localPhoneRadarMap, option);
    qlibc::QData finalRooms = getSubstitudeRoomsData(rooms, localRooms);
    
    payload.asValue()["info"]["devices"] = finalDevices.asValue();
    payload.asValue()["info"]["area_app"] = finalAreas.asValue();
    payload.asValue()["info"]["doors"] = finalDoors.asValue();
    payload.asValue()["info"]["rooms"] = finalRooms.asValue();
    payload.setString("timeStamp", std::to_string(localTime + 1));

    //保存设备列表
    qlibc::QData contentSaveRequest, contentSaveResponse;
    contentSaveRequest.setString("service_id", WHITELIST_SAVE_REQUEST_SERVICE_ID);
    contentSaveRequest.putData("request", payload);
    httpUtil::sitePostRequest("127.0.0.1", 9006, contentSaveRequest, contentSaveResponse);
}
#endif