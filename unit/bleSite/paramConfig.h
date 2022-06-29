//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAMCONFIG_H
#define EXHIBITION_PARAMCONFIG_H

#include <string>

using namespace std;

const int BleSitePort = 60009;

//服务ID
static const string Ble_Device_Command_Service_ID = "BleDeviceCommand";


//控制命令
#define SCAN                        "scan"
#define SCANEND                     "scanEnd"
#define CONNECT                     "connect"
#define ASSIGN_GATEWAY_ADDRESS      "assignGateWayAddress"
#define ASSIGN_NODE_ADDRESS         "assignNodeAddress"
#define BIND                        "bind"


#endif //EXHIBITION_PARAMCONFIG_H
