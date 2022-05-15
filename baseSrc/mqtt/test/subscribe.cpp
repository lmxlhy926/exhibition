
#include "mqtt/mqttClient.h"
#include "qlibc/QData.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <cstdio>
#include <cstring>

using namespace std;
using namespace qlibc;

int main(int argc, char* argv[]) {

    string fileName = argv[1];
    QData configData;
    configData.loadFromFile(fileName);
    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");

    mqttClient mc;
    mc.addDataHooker([](const std::string& topic, void *payload, int payloadLen, char* buffer, int* len) -> bool{

        string str = "welcome to addDataHooker";
        memcpy(buffer, str.data(), str.size());
        *len = str.size();
        return true;
    });
    mc.setTopicHandler("abc", [](const string& topic, char* payload, int len) ->bool{
        std::cout << "in setTopicHandler: topic<" << topic << ">---message: <" << string(payload, len) << ">....." << std::endl;
        return true;
    });

    mc.paramConfig(server, port, username, password, "subscribe");
    mc.connect();
    mc.subscribe("abc");

    while(true)
        std::this_thread::sleep_for(std::chrono::seconds(10));
}



















