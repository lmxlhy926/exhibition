//
// Created by 78472 on 2022/5/15.
//

#include "serviceRequestHandler.h"



int tvupload_service_handler(socketClient& client, const Request& request, Response& response){

    client.sendMessage("");
    return 0;
}

int sensor_service_handler(socketClient& client, const Request& request, Response& response){
    return 0;
}




