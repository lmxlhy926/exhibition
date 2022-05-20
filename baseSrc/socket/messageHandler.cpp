//
// Created by 78472 on 2022/5/4.
//

#include "messageHandler.h"

messageHandler::messageHandler() {
    defaultHandler_ = [](QData& message) -> bool{
        std::cout << "-->received default unhandled msg>>:" << message.toJsonString() << endl;
        return true;
    };
}

void messageHandler::setDefaultHandler(const JsonSocketHandler &defaultHandler) {
    std::lock_guard<std::mutex> lg(mutex_);
    defaultHandler_ = defaultHandler;
}

void messageHandler::setUriHandler(const string &uri, const JsonSocketHandler &jsHandler) {
    std::lock_guard<std::mutex> lg(mutex_);
    if(uriHandlers_.find(uri) != uriHandlers_.end())
        uriHandlers_.erase(uri);
    uriHandlers_.insert(std::make_pair(uri, jsHandler));
}

void messageHandler::disPatchMessage(const string &uri, QData &message) {
    std::lock_guard<std::mutex> lg(mutex_);
    auto it = uriHandlers_.find(uri);
    if(it != uriHandlers_.end()){
        it->second(message);
        return;
    }
    defaultHandler_(message);
}
