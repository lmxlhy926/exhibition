//
// Created by 78472 on 2022/10/22.
//

#include <thread>
#include <atomic>
#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"
#include "log/Logging.h"
#include "mqtt/mqttClient.h"
#include "util/secretUtils.h"

#include "handle/mqttHandle.h"
#include "handle/mdmConfig.h"
#include "mdmParam.hpp"
//#include "ssl/sslUtil.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;


int main(int argc, char* argv[]) {
    if(argc != 2){
        LOG_RED << "Usage Error.....";
        LOG_PURPLE << "Try again with the format: mdm_site <DirPath>";
        return 0;
    }

    // 设置线程池
    httplib::ThreadPool threadPool_(10);
    std::atomic<bool> http_server_thread_end(false);

    // 配置本站点启动信息
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(MDM_SITE_PORT);
    serviceSiteManager->setSiteIdSummary(MDM_SITE_ID, MDM_SITE_ID_NAME);

    // 注册支持的消息
    serviceSiteManager->registerMessageId(CALLING_MESSAGE_ID);

    //. 设置配置文件加载路径
    mdmConfig* configPathPtr = mdmConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    //添加ssl访问服务
//    string host = "open.ys7.com";
//    string authHost = "openauth.ys7.com";
//    string caPath = mdmConfig::getInstance()->getCaPath();
//    sslUtil::getInstance()->addCloudSite(EZVIZ, host, caPath);
//    sslUtil::getInstance()->addCloudSite(EZVIZAUTH, authHost, caPath);

    //. 获取mqtt配置参数
    QData mqttConfigData = mdmConfig::getInstance()->getMqttConfigData();
    std::string mqttServer = mqttConfigData.getString("server");
    int mqttPort = mqttConfigData.getInt("port");
    std::string mqttUsername = mqttConfigData.getString("username");
    std::string mqttPassword = mqttConfigData.getString("password");

    //加载基础参数信息
    QData baseInfoData = mdmConfig::getInstance()->getBaseInfoData();
    string domainID = baseInfoData.getString("domainID");
    string clientID = "b10mdm" + std::to_string(time(nullptr)) ;

    //配置mqtt客户端、设置处理回调、设置预处理回调、订阅主题
    mqttClient mc;
    mc.paramConfig(mqttServer, mqttPort, mqttUsername, mqttPassword, clientID);
    if(!domainID.empty()){
        mc.subscribe("edge/" + domainID + "/device/domainWhite");
    }
    mc.setDefaultHandler(mqttHandle::handle);
    mc.addDataHooker([](const std::string& topic, void *payload, int payloadLen, char* buffer, int* len)->bool{
        const string in = string(reinterpret_cast<char *>(payload), 0, payloadLen);
        string out;
        const uint8_t key[] = "123456asdfgh1234";
        lhytemp::secretUtil::ecb_decrypt_withPadding(in, out, key);

        strcpy(buffer, out.data());
        *len = static_cast<int>(out.size());

        return true;
    });
    mc.connect();


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
//            int code = serviceSiteManager->start();
            //注册启动方式
            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                LOG_RED << "===>mdmSite startByRegister error, code = " << code;
                LOG_RED << "===>mdmSite startByRegister in 3 seconds....";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                LOG_RED << "===>mdmSite startByRegister successfully.....";
                break;
            }
        }
    });


    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
