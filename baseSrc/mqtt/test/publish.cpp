
#include "mqtt/mqttClient.h"
#include "qlibc/QData.h"
#include <thread>
#include <chrono>
#include <iostream>

using namespace qlibc;

int main(int argc, char* argv[]) {

    QData configData;
    configData.loadFromFile(R"(D:\bywg\project\exhibition\baseSrc\mqtt\test\hongmeimqttconfig.json)");
    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");

    mqttClient mc;
    mc.paramConfig(server, port, username, password, "publish");
    mc.connect();

   while(true){
       std::this_thread::sleep_for(std::chrono::seconds(3));
//       mc.publish("abc", "hello");
   }

}



















