//
// Created by 78472 on 2022/5/8.
//

#ifndef EXHIBITION_MQTTCLIENT_H
#define EXHIBITION_MQTTCLIENT_H

#include <memory>
#include <mutex>
#include <functional>
#include "mqtt/include/MQTTAsync.h"
#include "mqttMessageHandler.h"
#include "qlibc/StringUtils.h"

using namespace std;

using MqttDataHooker = std::function<bool(const std::string& topic, void *payload, int payloadLen, char* buffer, int* len)>;

class mqttClient {
private:
    MQTTAsync client_ = nullptr;
    std::string mServerUrl;
    int port_;
    std::string mClientId;
    std::string mUserName;
    std::string mPassWd;
    std::recursive_mutex mutex_;

    bool mConnected = false;    //是否处于连接状态
    MqttDataHooker hooker;
    mqttMessageHandler messageHandler;

public:
    mqttClient();
    ~mqttClient() = default;

    void paramConfig(const string& server, int port,
                     const string& userName, const string& passWd,
                     const string& clientID);

//    bool connect();
//
//    bool publish(const string& topic, const string& msg, int Qos = 2);
//
//    bool subscribe(const string& topic, int QOs = 2);
//
//    void setDefaultHandler(const MqttMsgHandler& defaultHandler);
//
//    void setTopicHandler(const string& topic, const MqttMsgHandler & handler);
//
//    bool addDataHooker(MqttDataHooker& dataHooker);
//
//private:
//    static int onMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message);
//
//    void onMessageArrvd(char *topicName, int topicLen, void *payload, int payloadLen);

};


#endif //EXHIBITION_MQTTCLIENT_H
