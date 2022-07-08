//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_PARAMETER_H
#define EXHIBITION_PARAMETER_H

#include <string>

using namespace std;

const int BleSitePort = 60009;

//服务ID
static const string Ble_Device_Command_Service_ID = "BleDeviceCommand";

static const string Ble_Device_Test_Command_Service_ID = "BleDeviceCommands";


//控制命令
#define SCAN                        "scan"
#define SCANEND                     "scanEnd"
#define BIND                        "bind"

#define CONNECT                     "connect"
#define ASSIGN_GATEWAY_ADDRESS      "assignGateWayAddress"
#define ASSIGN_NODE_ADDRESS         "assignNodeAddress"



#endif //EXHIBITION_PARAMETER_H
