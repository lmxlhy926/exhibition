//
// Created by 78472 on 2022/5/11.
//

#include "subscribeMessageHandler.h"

void deviceStatus_message_handler(socketServer& server, const Request& request) {
   std::cout << "deviceStatus_message_handler: " << request.body << std::endl;
   server.postMessage(request.body + "\n");
}

void radar_message_handler(socketServer& server, const Request& request) {

}

void micpanel_message_handler(socketServer& server, const Request& request) {

}