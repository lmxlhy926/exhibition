//
// Created by 78472 on 2022/5/8.
//

#ifndef EXHIBITION_MQTTMESSAGEHANDLER_H
#define EXHIBITION_MQTTMESSAGEHANDLER_H

#include "qlibc/QData.h"
#include <functional>
#include <unordered_map>
#include <iostream>
#include <mutex>

using namespace std;

using MqttMsgHandler = std::function<bool(const string& topic, char* payload, int len)>;
using TopicHandlers = std::unordered_map<string, MqttMsgHandler>;

class mqttMessageHandler {
private:
    TopicHandlers topicHandlers_;
    MqttMsgHandler defaultHandler_;
    std::mutex mutex_;
public:
    explicit mqttMessageHandler();

    void setDefaultHandler(const MqttMsgHandler& defaultHandler);

    void setTopicHandler(const string& topic, const MqttMsgHandler & handler);

    void disPatchMessage(const string& topic, char* payload, int len);

};

#endif //EXHIBITION_MQTTMESSAGEHANDLER_H
