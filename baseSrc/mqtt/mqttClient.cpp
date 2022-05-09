//
// Created by 78472 on 2022/5/8.
//

#include "mqttClient.h"
#include <iostream>

mqttClient::mqttClient(){}

void mqttClient::paramConfig(const string &server, int port,
                             const string &userName, const string &passWd,
                             const string &clientID) {
    mServerUrl = StringUtils::formatString("tcp://%s:%d", server.c_str(), port);
    port_ = port;
    mUserName = userName;
    mPassWd = passWd;
    mClientId = clientID;
}

bool mqttClient::connect() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    if(mConnected)
        return true;
    if(client_ != nullptr)
        MQTTAsync_destroy(&client_);

    MQTTAsync_createOptions createOptions = MQTTAsync_createOptions_initializer;
    createOptions.sendWhileDisconnected = 1;
    createOptions.maxBufferedMessages = 32;

    MQTTAsync_createWithOptions(&client_, mServerUrl.c_str(), mClientId.c_str(),
                                MQTTCLIENT_PERSISTENCE_NONE, nullptr, &createOptions);

    MQTTAsync_setCallbacks(client_, this, nullptr, onMsgArrvd, nullptr);

    MQTTAsync_connectOptions connectOptions = MQTTAsync_connectOptions_initializer;
    connectOptions.keepAliveInterval = 30;
    connectOptions.cleansession = 1;
    connectOptions.username = mUserName.c_str();
    connectOptions.password = mPassWd.c_str();
    connectOptions.automaticReconnect = 1;
    connectOptions.minRetryInterval = 1;
    connectOptions.maxRetryInterval = 10;
    connectOptions.context = this;

    int rc = MQTTAsync_connect(client_, &connectOptions);
    if(rc != MQTTASYNC_SUCCESS){
        std::cout << "failed to strart connect: " << MQTTAsync_strerror(rc) << std::endl;
        MQTTAsync_destroy(&client_);
        client_ = nullptr;
        return false;
    }
    mConnected = true;
    return true;
}

bool mqttClient::publish(const string &topic, const string &msg, int Qos) {
    MQTTAsync_responseOptions responseOptions = MQTTAsync_responseOptions_initializer;

    MQTTAsync_message publishMessage = MQTTAsync_message_initializer;
    publishMessage.payload = (void *)(msg.c_str());
    publishMessage.payloadlen = static_cast<int>(msg.length());
    publishMessage.qos = Qos;
    publishMessage.retained = 0;

    int rc = MQTTAsync_sendMessage(client_, topic.c_str(), &publishMessage, &responseOptions);
    if(rc != MQTTASYNC_SUCCESS){
        std::cout  << "failed to publish the message: " << msg << "-->" << MQTTAsync_strerror(rc) << std::endl;
        return false;
    }
    std::cout << "the message: " << msg << "--->publish successfully" << std::endl;
    return true;
}

bool mqttClient::subscribe(const string &topic, int QOs) {
    MQTTAsync_responseOptions responseOptions = MQTTAsync_responseOptions_initializer;

    int rc = MQTTAsync_subscribe(client_, topic.c_str(), QOs, &responseOptions);
    if(rc != MQTTASYNC_SUCCESS){
        std::cout << "failed to subscribe topic <" << topic << ">, error: " << MQTTAsync_strerror(rc) << std::endl;
        return false;
    }
    std::cout << "the topic <" << topic << "> subscribe successfully" << std::endl;
    return true;
}

void mqttClient::setDefaultHandler(const MqttMsgHandler &defaultHandler) {
    messageHandler.setDefaultHandler(defaultHandler);
}

void mqttClient::setTopicHandler(const string& topic, const MqttMsgHandler & handler){
    messageHandler.setTopicHandler(topic, handler);
}

bool mqttClient::addDataHooker(MqttDataHooker& dataHooker){
    this->hooker = dataHooker;
}

int mqttClient::onMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    auto client = (mqttClient *)(context);
    client->onMessageArrvd(topicName, topicLen, message->payload, message->payloadlen);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}

void mqttClient::onMessageArrvd(char *topicName, int topicLen, void *payload, int payloadLen) {
    string topic(topicName, topicLen);
    if(hooker != nullptr){
        char buffer[64 * 1024]{};
        int size;
        hooker(topic, payload, payloadLen, buffer, &size);
        messageHandler.disPatchMessage(topic, buffer, size);
    }else{
        messageHandler.disPatchMessage(topic, reinterpret_cast<char *>(payload), payloadLen);
    }
}































