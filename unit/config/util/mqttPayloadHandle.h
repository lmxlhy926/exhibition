//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_MQTTPAYLOADHANDLE_H
#define EXHIBITION_MQTTPAYLOADHANDLE_H

#include <string>

using namespace std;

static const string WHITELIST_MESSAGE_ID = "whiteList";

class mqttPayloadHandle {
public:
    static bool handle(const string& topic, char* payloadReceive, int len);

};


#endif //EXHIBITION_MQTTPAYLOADHANDLE_H
