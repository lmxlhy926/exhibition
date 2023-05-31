
#include <thread>
#include <atomic>
#include <cstdio>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "serviceRequestHandler.h"
#include "qlibc/FileUtils.h"
#include "util/mqttPayloadHandle.h"
#include "util/secretUtils.h"
#include "param.h"
#include "log/Logging.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    //增加log打印
    string path = "/data/changhong/edge_midware/lhy/configSiteLog.txt";
    muduo::logInitLogger(path);

    LOG_RED << "-----------------------------------------";
    LOG_RED << "-----------------------------------------";
    LOG_RED << "---------------CONFIG_ITE START----------";
    LOG_RED << "-----------------------------------------";
    LOG_RED << "-----------------------------------------";

    for(int i = 0; i < argc; ++i){
        LOG_PURPLE << "ARGPRINT......";
        LOG_RED << argv[i];
    }

    if(argc != 2 && argc !=3 ){
        LOG_RED << "Usage Error.....";
        LOG_PURPLE << "Try again with the format: [config <DirPath>], [config <DirPath> <--RK3308>]";
        return 0;
    }

    bool ISRK3308 = false;
    if(argc == 3 && string(argv[2]) == "--RK3308"){
        ISRK3308 = true;
        LOG_PURPLE << "RK3308.........";
    }else{
        LOG_PURPLE << "SMART HOME......";
    }

    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(ConfigSitePort);
    serviceSiteManager->setSiteIdSummary(CONFIG_SITE_ID, CONFIG_SITE_ID_NAME);
    httplib::ThreadPool threadPool_(10);
    //设置配置文件加载路径
    configParamUtil* configPathPtr = configParamUtil::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));


    mqttClient mc;
    if(!ISRK3308){
        //初始化cloudUtil, 加载http服务器配置信息
        QData httpConfigData = configPathPtr->getCloudServerData();
        const string dataDirPath = configPathPtr->getconfigPath();
        cloudUtil::getInstance()->init(httpConfigData.getString("ip"), httpConfigData.getInt("port"), dataDirPath);

        //开启线程，阻塞进行电视加入大白名单
        threadPool_.enqueue([](){
            cloudUtil::getInstance()->joinTvWhite();
        });

        //. 获取mqtt配置参数
        QData mqttConfigData = configParamUtil::getInstance()->getMqttConfigData();
        std::string mqttServer = mqttConfigData.getString("server");
        int mqttPort = mqttConfigData.getInt("port");
        std::string mqttUsername = mqttConfigData.getString("username");
        std::string mqttPassword = mqttConfigData.getString("password");

        //加载基础参数信息
        QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
        string domainID = baseInfoData.getString("domainID");
        string clientID = domainID.empty() ?
                          "config" + std::to_string(time(nullptr)) : "config" + domainID + std::to_string(time(nullptr));

        //配置mqtt客户端、设置处理回调、设置预处理回调、订阅主题
        mc.paramConfig(mqttServer, mqttPort, mqttUsername, mqttPassword, clientID);
        if(!domainID.empty()){
            mc.subscribe("edge/" + domainID + "/device/domainWhite");
        }
        mc.setDefaultHandler(mqttPayloadHandle::handle);
        mc.addDataHooker([](const std::string& topic, void *payload, int payloadLen, char* buffer, int* len)->bool{
            LOG_PURPLE << "payloadLen: " << payloadLen;
            const string in = string(reinterpret_cast<char *>(payload), 0, payloadLen);
            string out;
            const uint8_t key[] = "123456asdfgh1234";
            lhytemp::secretUtil::ecb_decrypt_withPadding(in, out, key);

            strcpy(buffer, out.data());
            *len = static_cast<int>(out.size());
            LOG_PURPLE << "out.size(): " << out.size();

            return true;
        });
        mc.connect();

        //注册请求场景列表处理函数
        serviceSiteManager->registerServiceRequestHandler(SCENELIST_REQUEST_SERVICE_ID,[&](const Request& request, Response& response)->int{
            return sceneListRequest_service_request_handler(request, response, mc.isConnected());
        });

        //注册子设备注册处理函数
        serviceSiteManager->registerServiceRequestHandler(SUBDEVICE_REGISTER_SERVICE_ID, [&](const Request& request, Response& response)->int{
            return subDeviceRegister_service_request_handler(request, response, mc.isConnected());
        });

        //设备列表同步
        serviceSiteManager->registerServiceRequestHandler(POSTDEVICELIST_REGISTER_SERVICE_ID, [&](const Request& request, Response& response)->int{
            return postDeviceList_service_request_handler(request, response, mc.isConnected());
        });

        //注册获取家庭域Id处理函数
        serviceSiteManager->registerServiceRequestHandler(DOMAINID_REQUEST_SERVICE_ID, [&](const Request& request, Response& response)->int{
            return domainIdRequest_service_request_handler(request, response, mc.isConnected());
        });

        //注册安装师傅信息上传请求函数
        serviceSiteManager->registerServiceRequestHandler(ENGINEER_REQUEST_SERVICE_ID,[&](const Request& request, Response& response) -> int{
            return engineer_service_request_handler(mc, request, response);
        });

        //从云端主动获取白名单
        serviceSiteManager->registerServiceRequestHandler(WHITELISTCLOUD_REQUEST_SERVICE_ID,[&](const Request& request, Response& response) -> int{
            return getWhiteListFromCloud_service_request_handler(mc, request, response);
        });
    }


    //获取白名单列表
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_REQUEST_SERVICE_ID, whiteList_get_service_request_handler);

    //保存白名单列表
    serviceSiteManager->registerServiceRequestHandler(WHITELIST_SAVE_REQUEST_SERVICE_ID, whiteList_save_service_request_handler);

    //获取场景配置文件
    serviceSiteManager->registerServiceRequestHandler(GET_SCENECONFIG_FILE_REQUEST_SERVICE_ID, getSceneFile_service_request_handler);

    //保存场景配置文件
    serviceSiteManager->registerServiceRequestHandler(SAVE_SCENECONFIG_FILE_REQUEST_SERVICE_ID, saveSceneFile_service_request_handler);

    //获取灯控设备列表
    serviceSiteManager->registerServiceRequestHandler(GETALLLIST_REQUEST_SERVICE_ID, getAllDeviceList_service_request_handler);

    //配置面板配置信息
    serviceSiteManager->registerServiceRequestHandler(SETPANELINFO_REQUEST_SERVICE_ID, setPanelInfo_service_request_handler);

    //获取面板配置信息
    serviceSiteManager->registerServiceRequestHandler(GETPANELINFO_REQUEST_SERVICE_ID, getPanelInfo_service_request_handler);

    //保存语音面板设备
    serviceSiteManager->registerServiceRequestHandler(SETAUDIOPANELLIST_REQUEST_SERVICE_ID, saveAudioPanelList_service_request_handler);

    //获取语音面板设备
    serviceSiteManager->registerServiceRequestHandler(GETAUDIOPANELLIST_REQUEST_SERVICE_ID, getAudioPanelList_service_request_handler);

    //设置雷达信息
    serviceSiteManager->registerServiceRequestHandler(SETRADARLIST_REQUEST_SERVICE_ID, setRadarDevice_service_request_handler);


    //set site supported subscribed message
    serviceSiteManager->registerMessageId(WHITELIST_MESSAGE_ID);                //发布白名单给第三方
    serviceSiteManager->registerMessageId(RECEIVED_WHITELIST_ID);               //发布消息，告知已接收到白名单
    serviceSiteManager->registerMessageId(WHITELIST_MODIFIED_MESSAGE_ID);       //发布消息，告知白名单已被修改
    serviceSiteManager->registerMessageId(SCENELIST_MODIFIED_MESSAGE_ID);       //发布消息，告知场景文件已被修改
    serviceSiteManager->registerMessageId(PANELINFO_MODIFIED_MESSAGE_ID);       //发布消息，面板配置信息更改
    serviceSiteManager->registerMessageId(WHITELIST_MERGE_MESSAGE_ID);          //发布消息，通知面板进行白名单合并
    serviceSiteManager->registerMessageId(SCENEFILE_UPDATE_MESSAGE_ID);         //发布消息，通知面板进行白名单更新

    servicesite::ServiceSiteManager::registerMessageHandler(WHITELIST_MERGE_MESSAGE_ID, messageTrigger);
    servicesite::ServiceSiteManager::registerMessageHandler(SCENEFILE_UPDATE_MESSAGE_ID, messageTrigger);


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
        //    int code = serviceSiteManager->start();
            //注册启动方式
            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                LOG_RED << "===>configSite startByRegister error, code = " << code;
                LOG_RED << "===>configSite startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_RED << "===>configSite startByRegister successfully.....";
                break;
            }
        }
    });

    //30秒同步一次白名单和场景文件
    std::this_thread::sleep_for(std::chrono::seconds(2));
    threadPool_.enqueue([&](){
        while(true){
            //订阅消息
            std::vector<string> messageIdList;
            messageIdList.push_back(WHITELIST_MERGE_MESSAGE_ID);
            messageIdList.push_back(SCENEFILE_UPDATE_MESSAGE_ID);
            subscribeFromAllConfigSite(messageIdList);
            //更新文件
            whiteListFileSync(CONFIG_SITE_ID, WHITELIST_REQUEST_SERVICE_ID, "auto update");          //同步白名单
            sceneFileSync(CONFIG_SITE_ID, GET_SCENECONFIG_FILE_REQUEST_SERVICE_ID, "auto update");   //同步场景文件
            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    });


    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
