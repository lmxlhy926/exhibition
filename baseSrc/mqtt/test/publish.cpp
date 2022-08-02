
#include "mqtt/mqttClient.h"
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace qlibc;

static const char* configPath = "/cygdrive/d/project/byjs/exhibition/baseSrc/mqtt/test";


int main(int argc, char* argv[]) {

    string fileName = FileUtils::contactFileName(configPath, "hongmeimqttconfig.json");
    std::cout << "loadFileName: " << fileName << std::endl;
    QData configData;
    configData.loadFromFile(fileName);
    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");

    mqttClient mc;
    mc.paramConfig(server, port, username, password, "publish");
    mc.connect();

   while(true){
       std::this_thread::sleep_for(std::chrono::seconds(3));
       mc.publish("abc", "hello");
   }

}



















