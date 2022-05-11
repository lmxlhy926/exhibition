//
// Created by 78472 on 2022/5/4.
//

#ifndef EXHIBITION_MESSAGEHANDLER_H
#define EXHIBITION_MESSAGEHANDLER_H

#include "qlibc/QData.h"
#include <functional>
#include <unordered_map>
#include <iostream>
#include <mutex>


using namespace std;
using namespace qlibc;

using JsonSocketHandler = std::function<bool(QData& message)>;
using UriHandlers = std::unordered_map<string, JsonSocketHandler>;

class messageHandler {
private:
    UriHandlers uriHandlers_;
    JsonSocketHandler defaultHandler_;
    std::mutex mutex_;
public:
    explicit messageHandler();

    void setDefaultHandler(const JsonSocketHandler& defaultHandler);

    void setUriHandler(const string& uri, const JsonSocketHandler& jsHandler);

    void disPatchMessage(const string& uri, QData& message);

};


#endif //EXHIBITION_MESSAGEHANDLER_H
