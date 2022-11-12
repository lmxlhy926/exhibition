
#include "mqtt/mqttClient.h"
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace qlibc;


int main(int argc, char* argv[]) {
    QData configData;
    configData.loadFromFile("/mnt/d/bywg/project/exhibition/baseSrc/zbaseTest/mqttTest/mqttconfig.json");
    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");
    std::cout << "config: " << configData.toJsonString(true);


    mqttClient mc;
    mc.paramConfig(server, port, username, password, "publish");
    mc.connect();

    qlibc::QData content;
    content.loadFromFile("/mnt/d/bywg/project/exhibition/unit/zunitTest/bleTest/originWhiteList.json");


    std::this_thread::sleep_for(std::chrono::seconds(2));
    mc.publish("edge/did:chisid:0x52e2bb770ec34307bc0f6120f6c53ab27d280c95/device/domainWhite", content.toJsonString());
}





















