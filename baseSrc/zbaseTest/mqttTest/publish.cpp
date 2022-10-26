
#include "mqtt/mqttClient.h"
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace qlibc;


int main(int argc, char* argv[]) {

    QData configData;
    configData.loadFromFile(R"(D:\project\byjs\exhibition\baseSrc\mqtt\test\mqttconfig.json)");
    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");

    mqttClient mc;
    mc.paramConfig(server, port, username, password, "publish");
    mc.connect();

    qlibc::QData content;
    content.loadFromFile(R"(D:\project\byjs\exhibition\baseSrc\mqtt\test\a.json)");

    std::this_thread::sleep_for(std::chrono::seconds(2));
    mc.publish("publish", content.toJsonString());
}



















