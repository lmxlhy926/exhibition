

#include <iostream>
#include <string>
#include "socket/httplib.h"
#include "qlibc/QData.h"

int main(int argc, char* argv[]){

    string on = R"({"service_id":"BleDeviceCommand","request":{"command":"turnOn","device_id":"FFFF","status_value":"on"}})";
    string off = R"({"service_id":"BleDeviceCommand","request":{"command":"turnOff","device_id":"FFFF","status_value":"on"}})";

    httplib::Client client("127.0.0.1", 60000);
    for(int i = 0; i < 1000 * 10; i++){
        if( i % 2 == 0){
            httplib::Result result =  client.Post("/turnOn", on, "text/json");
            if(result != nullptr){
                printf("==>response: %s\n", result.value().body.c_str());
            }
        }else{
            httplib::Result result =  client.Post("/turnOff", off, "text/json");
            if(result != nullptr){
                printf("==>response: %s\n", result.value().body.c_str());
            }
        }

    }

    return 0;
}
















