

#include <iostream>
#include <string>
#include "http/httplib.h"
#include "log/Logging.h"
#include "qlibc/QData.h"

int main(int argc, char* argv[]){

    string on = R"({"service_id":"control_device","request":{"device_list":[{"device_id":"A8270F971B30","command_list":[{"command_id":"power","command_para":"on"}]}]}})";
    string off = R"({"service_id":"control_device","request":{"device_list":[{"device_id":"A8270F971B30","command_list":[{"command_id":"power","command_para":"off"}]}]}})";


    httplib::Client client("127.0.0.1", 60009);
    for(int i = 0; i < 60; i++){
        if( i % 2 == 0){
            httplib::Result result =  client.Post("/", on, "text/json");
            LOG_INFO << "--->ON.....";

        }else{
            httplib::Result result =  client.Post("/", off, "text/json");
            LOG_RED << "--->OFF.....";
        }

    }

    return 0;
}
















