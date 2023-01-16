
#include "mqtt/mqttClient.h"
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace qlibc;

#define TOPIC               "edge/lhy/device/domainWhite"
#define CLIENTID            "lhypublish"
#define CONFIGPAHT          "/mnt/d/bywg/project/exhibition/baseSrc/zbaseTest/mqttTest/mqttconfig.json"
#define CONTENTFILEPATH     "/mnt/d/bywg/project/exhibition/baseSrc/zbaseTest/mqttTest/publish.json"


int main(int argc, char* argv[]) {
    QData configData;
    configData.loadFromFile(CONFIGPAHT);
    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");
    std::cout << "config: " << configData.toJsonString(true);

    //连接
    mqttClient mc;
    mc.paramConfig(server, port, username, password, CLIENTID);
    mc.connect();

    //加载文件内容
    qlibc::QData content;
    content.loadFromFile(CONTENTFILEPATH);

    //发布
    std::this_thread::sleep_for(std::chrono::seconds(2));
    mc.publish(TOPIC, content.toJsonString());
}





















