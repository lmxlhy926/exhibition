
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "socket/httplib.h"
#include "common/configParamUtil.h"
#include "qlibc/FileUtils.h"
#include "serviceRequestHandler.h"
#include "socket/socketClient.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string SYNERGY_SITE_ID = "collaborate";
static const string SYNERGY_SITE_ID_NAME = "协同站点";


void publish_message(void){
//    json message_json = {
//            {"message_id", "test_message_id_1"},
//            {"content", {
//                                   "some_data", 123
//                           }}
//    };
//
//    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
//
//    // 把要发布的消息 json 字符串传入即可， 由库来向订阅过的站点发送消息
//    serviceSiteManager->publishMessage(TEST_MESSAGE_ID_1, message_json.dump());
//
//    message_json["message_id"] = "test_message_id_2";
//    serviceSiteManager->publishMessage(TEST_MESSAGE_ID_2, message_json.dump());
}



int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(100);
    std::atomic<bool> http_server_thread_end(false);

    //设置配置文件加载路径, 加载配置文件
    configParamUtil* configPathPtr = configParamUtil::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));
    QData interActiveAppData = configPathPtr->getInterActiveAppData();
    string activeAppServerIp = interActiveAppData.getString("host");
    int activeAppPort1 = interActiveAppData.getInt("port1");
    int activeAppPort2 = interActiveAppData.getInt("port2");

    //创建socketClient1
    string loginMessage = "{\"identity\":\"ctl\",\"pwd\":\"ctl123456\"}\n";
    socketClient socket_client_1(threadPool_);
    socket_client_1.setAfterConnectHandler([](){
        //todo 获取tvMac
    });
    socket_client_1.setUriHandler("/dev/deviceControl", [](QData& message)->bool{
        //todo 格式转换
    });
    socket_client_1.start(activeAppServerIp, activeAppPort1, loginMessage);



    socketClient socket_client_2(threadPool_);
    socket_client_2.setAfterConnectHandler([&](){
        //todo 获取tvMac
    });
    socket_client_2.start(activeAppServerIp, activeAppPort2, loginMessage);



    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();

    // 注册 Service 请求处理 handler， 有两个 Service
    serviceSiteManager->registerServiceRequestHandler(TVUPLOAD_SERVICE_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return tvupload_service_handler(socket_client_1, request, response);
    });

    serviceSiteManager->registerServiceRequestHandler(SENSOR_SERVICE_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return sensor_service_handler(socket_client_1, request, response);
    });


#if 0
    // 注册支持的消息ID
    serviceSiteManager->registerMessageId(DEVICESTATUS_MESSAGE_ID);
    // 注册消息ID对应的handler
    serviceSiteManager->registerMessageHandler(DEVICESTATUS_MESSAGE_ID,
                                               [&](const Request& request){
        deviceStatus_message_handler(server, request);
    });
#endif



    // 站点监听线程启动
    threadPool_.enqueue([&](){
        // 启动服务器，参数为端口， 可用于单独的开发调试
        int code = serviceSiteManager->start(60002);

        // 通过注册的方式启动服务器， 需要提供site_id, site_name, port
//    	code = serviceSiteManager->startByRegister(TEST_SITE_ID_1, TEST_SITE_NAME_1, 9001);

        if (code != 0) {
            printf("start error. code = %d\n", code);
        }

        http_server_thread_end.store(true);
    });

    sleep(2);

    if (http_server_thread_end.load()) {
        printf("启动 http 服务器线程错误.\n");
        return -1;
    }

#if 0
    // 订阅消息, 需要传入订阅站点的IP、端口号、消息ID列表
    std::vector<string> messageIdList;
    messageIdList.push_back(DEVICESTATUS_MESSAGE_ID);
    int  code = serviceSiteManager->subscribeMessage("127.0.0.1", 60001, messageIdList);
    if (code == ServiceSiteManager::RET_CODE_OK) {
        printf("subscribeMessage ok.\n");
    }
#endif

    while(true){
        if (http_server_thread_end){
            printf("http end abnormally....\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

    return -1;
}
