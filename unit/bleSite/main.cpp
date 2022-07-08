
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>
#include <sstream>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "formatTrans/bleConfig.h"
#include "qlibc/FileUtils.h"
#include "formatTrans/binary2JsonEvent.h"
#include "serviceRequestHandler.h"
#include "parameter.h"
#include "formatTrans/lightUpStatus.h"

using namespace std;
using namespace servicesite;
using namespace httplib;
using namespace std::placeholders;
using json = nlohmann::json;

static const string SYNERGY_SITE_ID = "BLE";
static const string SYNERGY_SITE_ID_NAME = "BLE";

int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(10);
    std::atomic<bool> http_server_thread_end(false);

    //初始化单例EventTable单例对象
//    EventTable::getInstance();

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(BleSitePort);

    //设置配置文件加载路径, 加载配置文件
    bleConfig* configPathPtr = bleConfig::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    //打开串口
    while(!configPathPtr->serialInit(Binary2JsonEvent::binary2JsonEvent)){
        std::cout << "==>failed to open the serial<"
                  << configPathPtr->getSerialData().getString("serial") << ">...." << std::endl;
        std::cout << "===>try to open in 3 seconds....." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    std::cout << "===>success in open serial<"
              << configPathPtr->getSerialData().getString("serial") << ">...." << std::endl;

    //注册蓝牙命令handler
    serviceSiteManager->registerServiceRequestHandler(Ble_Device_Command_Service_ID,
                                                      [&](const Request& request, Response& response) -> int{
        return BleDevice_command_service_handler(request, response);
    });

    serviceSiteManager->registerServiceRequestHandler(Ble_Device_Test_Command_Service_ID,
                                                      [&](const Request& request, Response& response) -> int{
                                                          return BleDevice_command_test_service_handler(request, response);
                                                      });


    // 站点监听线程启动
    threadPool_.enqueue([&](){
        while(true){
            //自启动方式
            int code = serviceSiteManager->start();
            //注册启动方式
//            int code = serviceSiteManager->startByRegister();
            if(code != 0){
                std::cout << "===>bleSite startByRegister error, code = " << code << std::endl;
                std::cout << "===>bleSite startByRegister in 3 seconds...." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }else{
                std::cout << "===>bleSite startByRegister successfully....." << std::endl;
                break;
            }
        }
    });

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(60 *10));
    }

    return 0;
}
