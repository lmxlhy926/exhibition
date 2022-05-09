
#include "mqtt/mqttClient.h"
#include "qlibc/QData.h"
#include <thread>
#include <chrono>
#include <iostream>

int main(int argc, char* argv[]) {

    QData configData;
    configData.loadFromFile(R"(D:\project\byjs\zhanting\exhibition\baseSrc\mqtt\test\mqttconfig.json)");
    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");

    mqttClient mc;
    mc.paramConfig(server, port, username, password, "publish");
    if(mc.connect()){
        std::cout << "----publish connect successfully---" << std::endl;
    }

   while(true){
       std::this_thread::sleep_for(std::chrono::seconds(3));
       mc.publish("abc", "hello");
   }

}



















