#include <iostream>
#include "common/httplib.h"
#include "qlibc/QData.h"
#include "log/Logging.h"
#include "serial/BLETelinkDongle.h"


//unsigned char test[6] = {0x01, 0xE9, 0xFF, 0x02, 0x10, 0x03};
unsigned char test[3] = {0xE9, 0xFF, 0x00};
unsigned char tmp_buf[256] = {0x00};

unsigned char cmd_on[14] = {0xE8,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x82,0x02,0x01,0x00};
unsigned char cmd_off[14] = {0xE8,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x82,0x02,0x00,0x00};

httplib::Server svr;
BLETelinkDongle *telinkD;

bool tempPrint(unsigned char *binaryStream, int size){
    stringstream ss;
    for (int i = 0; i < size; i++) {
        ss << std::setw(2) << std::setfill('0') << std::hex << std::uppercase << static_cast<int>(binaryStream[i]);
    }
    LOG_INFO << ss.str();
}


void cmd(const httplib::Request &req, httplib::Response &res){

    qlibc::QData requestBody(req.body);
    string command = requestBody.getData("request").getString("command");
    LOG_INFO << "command: " << command;

    if(command == "scan"){
        unsigned char cmd_data[3] = {0xE9,0xFF,0x00};
        telinkD->sendData(cmd_data, 3);
        LOG_HLIGHT << "SCAN....";
    }else{
        unsigned char cmd_data[3] = {0xE9,0xFF,0x01};
        telinkD->sendData(cmd_data, 3);
        LOG_RED << "SCANEND.....";
    }

    res.set_content("ok", "text/plain");
    res.status = 200;
}

void startHttpService()
{
    svr.Post("/cmd", cmd);
    svr.listen("0.0.0.0", 60009);
}

int main() {
    std::cout << "start test......." << std::endl;

    telinkD = new BLETelinkDongle("ttyS2");
    telinkD->initDongle();
    telinkD->regRecvDataProc(tempPrint);
    bool ret = telinkD->startDongle();
    if(!ret)
        printf("startDongle() \n");

    std::thread httpservice(startHttpService);
    httpservice.detach();

    while(true){
        this_thread::sleep_for(std::chrono::seconds(100));
    }

    return 0;
}



void turnOnOffTest(){
    std::string on = R"({"service_id":"BleDeviceCommand","request":{"command":"turnOn","device_id":"FFFF","status_value":"on"}})";
    std::string off = R"({"service_id":"BleDeviceCommand","request":{"command":"turnOff","device_id":"FFFF","status_value":"on"}})";

    httplib::Client client("127.0.0.1", 60009);
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
}
