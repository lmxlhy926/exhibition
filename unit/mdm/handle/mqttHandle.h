//
// Created by 78472 on 2022/10/22.
//

#ifndef EXHIBITION_MQTTHANDLE_H
#define EXHIBITION_MQTTHANDLE_H

#include <string>
using namespace std;

class mqttHandle {
public:
    static bool handle(const string& topic, char* payloadReceive, int len);
};


#endif //EXHIBITION_MQTTHANDLE_H
