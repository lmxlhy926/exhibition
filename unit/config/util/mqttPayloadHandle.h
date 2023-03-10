//
// Created by 78472 on 2022/5/15.
//

#ifndef EXHIBITION_MQTTPAYLOADHANDLE_H
#define EXHIBITION_MQTTPAYLOADHANDLE_H

#include <string>
#include "qlibc/QData.h"


using namespace std;



class mqttPayloadHandle {
public:
    static qlibc::QData transform(const char* payloadReceive, int len);

    static bool handle(const string& topic, char* payloadReceive, int len);
};


#endif //EXHIBITION_MQTTPAYLOADHANDLE_H
