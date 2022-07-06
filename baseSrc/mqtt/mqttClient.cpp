//
// Created by 78472 on 2022/5/8.
//

#include "mqttClient.h"
#include <iostream>
#include <thread>
#include <chrono>

mqttClient::mqttClient(){}

mqttClient::~mqttClient(){
    if(client_ != nullptr)
        MQTTAsync_destroy(&client_);
}

void mqttClient::paramConfig(const string &server, int port,
                             const string &userName, const string &passWd,
                             const string &clientID) {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    mServerUrl = StringUtils::formatString("tcp://%s:%d", server.c_str(), port);
    port_ = port;
    mUserName = userName;
    mPassWd = passWd;
    mClientId = clientID;
    if(client_ != nullptr){
        MQTTAsync_destroy(&client_);
    }
    MQTTAsync_createOptions createOptions = MQTTAsync_createOptions_initializer;
    //createOptions.sendWhileDisconnected = 1;
    //createOptions.maxBufferedMessages = 32;

    MQTTAsync_createWithOptions(&client_, mServerUrl.c_str(), mClientId.c_str(),
                                MQTTCLIENT_PERSISTENCE_NONE, nullptr, &createOptions);

    MQTTAsync_setCallbacks(client_, this, connlost, onMsgArrvd, nullptr);
}

void mqttClient::connect() {
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    std::cout << "start to connect to mqttServer...." << std::endl;
    MQTTAsync_connectOptions connectOptions = MQTTAsync_connectOptions_initializer;
    connectOptions.keepAliveInterval = 10;
    connectOptions.cleansession = 1;
    connectOptions.username = mUserName.c_str();
    connectOptions.password = mPassWd.c_str();
    connectOptions.automaticReconnect = 1;  //自动重连
    connectOptions.minRetryInterval = 1;
    connectOptions.maxRetryInterval = 10;
    connectOptions.context = this;
    connectOptions.onSuccess = onConnect;
    connectOptions.onFailure = onConnectFailure;

    MQTTAsync_connect(client_, &connectOptions);
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
    std::lock_guard<std::recursive_mutex> lg(mutex_);

    auto ret = topicMap.find(topic);
    if(ret == topicMap.end())
        topicMap.insert(std::make_pair(topic, QOs));

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

bool mqttClient::addDataHooker(MqttDataHooker dataHooker){
    std::lock_guard<std::recursive_mutex> lg(mutex_);
    this->hooker = dataHooker;
    return true;
}

int mqttClient::onMsgArrvd(void *context, char *topicName, int topicLen, MQTTAsync_message *message) {
    std::cout << "onMsgArrvd: <" << topicName << ">---payloadLen<" << message->payloadlen << ">..." << std::endl;
    auto client = (mqttClient *)(context);
    client->onMsgArrvd_member(topicName, topicLen, message->payload, message->payloadlen);
    MQTTAsync_freeMessage(&message);
    MQTTAsync_free(topicName);
    return 1;
}


void mqttClient::connlost(void *context, char *cause) {
    auto client = (mqttClient *)(context);
    client->connlost_member(context, cause);
}

void mqttClient::onConnect(void *context, MQTTAsync_successData *response) {
    auto client = (mqttClient *)(context);
    client->onConnect_member(context, response);
}

void mqttClient::onConnectFailure(void *context, MQTTAsync_failureData *response) {
    auto client = (mqttClient *)(context);
    client->onConnectFailure_member(context, response);
}

void mqttClient::onMsgArrvd_member(char *topicName, int topicLen, void *payload, int payloadLen) {
    string topic(topicName, topicLen);
    if(hooker != nullptr){
        char* buffer = new char[MAX_MESSAGE_SIZE];
        int size;
        hooker(topic, payload, payloadLen, buffer, &size);
        messageHandler.disPatchMessage(topic, buffer, size);
        delete[] buffer;
    }else{
        messageHandler.disPatchMessage(topic, reinterpret_cast<char *>(payload), payloadLen);
    }
}

void mqttClient::connlost_member(void *context, char *cause) {
    std::cout << "-----connlost, start to connect aggain----" << std::endl;
    connect();
}

void mqttClient::onConnect_member(void *context, MQTTAsync_successData *response) {
    std::cout << "mqtt connect successfully....." << std::endl;
    for(auto& elem : topicMap){
        subscribe(elem.first, elem.second);
    }
}

void mqttClient::onConnectFailure_member(void *context, MQTTAsync_failureData *response) {
    std::cout << "onConnectFailure_member connect failed......." << std::endl;
}





