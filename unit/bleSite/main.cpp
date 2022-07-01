
#include <thread>
#include <unistd.h>
#include <atomic>
#include <cstdio>
#include <functional>
#include <sstream>

#include "siteService/nlohmann/json.hpp"
#include "siteService/service_site_manager.h"

#include "formatTrans/bleConfigParam.h"
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

bool serialReceive(unsigned char *data, int len){
    std::cout << "===>before decrypt>: ";
    for(int i = 0; i < len; i++)
        printf("%02X ", data[i]);
    printf("\n######################\n");

    stringstream ss;

    for( int i = 0; i < len; i++){
        ss << std::setw(2) << std::setfill('0') << std::hex << data[i] << " ";
    }
    ss << std::endl;
    printf("%s\n", ss.str().c_str());
    std::cout << ss.str() << std::endl;

    if(len > 512)   return true;
    int index = 0;
    unsigned char buf[512];
    memset(buf, 0, 512);

    if(data[0] == 0x01 && data[len - 1] == 0x03){
        for(int i = 1; i < len - 1; i++){
            if(data[i] == 0x02){
                buf[index++] = data[i + 1] & 0x0f;
                i = i + 1;
            }else{
                buf[index++] = data[i];
            }
        }
    }

    std::cout << "===>after decrypt>: ";
    for( int i = 0; i < index; i++){
        std::cout << std::setw(2) << std::setfill('0') << std::hex << buf[i] << " ";
    }
    std::cout << std::endl;
    return true;
}

int main(int argc, char* argv[]) {

    httplib::ThreadPool threadPool_(30);
    std::atomic<bool> http_server_thread_end(false);

    // 创建 serviceSiteManager 对象, 单例
    ServiceSiteManager* serviceSiteManager = ServiceSiteManager::getInstance();
    serviceSiteManager->setServerPort(BleSitePort);

    //设置配置文件加载路径, 加载配置文件
    bleConfigParam* configPathPtr = bleConfigParam::getInstance();
    configPathPtr->setConfigPath(string(argv[1]));

    //打开串口
    while(!configPathPtr->serialInit(serialReceive)){
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
