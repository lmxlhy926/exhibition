//
// Created by 78472 on 2022/5/8.
//

#include "mqttMessageHandler.h"

mqttMessageHandler::mqttMessageHandler() {
    defaultHandler_ = [](const string& topic, char* payload, int payloadLength) -> bool{
       std::cout << "default Handler-->topic: " << topic << "--->message: " << string(payload, payloadLength) << std::endl;
       return true;
    };
}

void mqttMessageHandler::setDefaultHandler(const MqttMsgHandler &defaultHandler) {
    std::lock_guard<std::mutex> lg(mutex_);
    defaultHandler_ = defaultHandler;
}

void mqttMessageHandler::setTopicHandler(const string &topic, const MqttMsgHandler &handler) {
    std::lock_guard<std::mutex> lg(mutex_);
    if(topicHandlers_.find(topic) != topicHandlers_.end())
        topicHandlers_.erase(topic);
    topicHandlers_.insert(std::make_pair(topic, handler));
}

void mqttMessageHandler::disPatchMessage(const string& topic, char* payload, int len) {
    std::lock_guard<std::mutex> lg(mutex_);
    auto it = topicHandlers_.find(topic);
    if(it != topicHandlers_.end()){
        it->second(topic, payload, len);
        return;
    }
    defaultHandler_(topic, payload, len);
}
