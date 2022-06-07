
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "bleConfigParam.h"
#include "qlibc/FileUtils.h"
#include "serviceRequestHandler.h"
#include "paramConfig.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string SYNERGY_SITE_ID = "BLE";
static const string SYNERGY_SITE_ID_NAME = "BLE";


int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    //设置配置文件加载路径, 加载配置文件
    bleConfigParam* configPathPtr = bleConfigParam::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(BleSitePort);

    // 注册 Service 请求处理 handler
    serviceSiteManager->registerServiceRequestHandler(Ble_Device_Service_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return BleDevice_control_service_handler(request, response);
    });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        // 启动服务器，参数为端口， 可用于单独的开发调试
        int code = serviceSiteManager->start();

        // 通过注册的方式启动服务器， 需要提供site_id, site_name, port
        //code = serviceSiteManager->startByRegister(TEST_SITE_ID_1, TEST_SITE_NAME_1, 9001);

        if (code != 0) {
            printf("start error. code = %d\n", code);
        }

        http_server_thread_end.store(true);
    });

    std::this_thread::sleep_for(std::chrono::seconds(3));

    while(true){
        if (http_server_thread_end){
            printf("http end abnormally....\n");
            break;
        }
        std::this_thread::sleep_for(std::chrono::seconds(30));
    }

    return -1;
}
