

#include <iostream>
#include <string>
#include "common/httplib.h"
#include "common/httpUtil.h"
#include "log/Logging.h"
#include "qlibc/QData.h"

#include "../serviceRequestHandler.h"


void commandRequest(){
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
}

void updateDeviceList(const Request& request) {
    //获取白名单列表
    qlibc::QData whiteListRequest, whiteListResponse;
    whiteListRequest.setString("service_id", "whiteListRequest");
    whiteListRequest.putData("request", qlibc::QData());

    SiteRecord::getInstance()->addSite("config", "127.0.0.1", 9006);
    SiteRecord::getInstance()->sendRequest2Site("config", whiteListRequest, whiteListResponse);
    if (whiteListResponse.getInt("code") == 0) {
        qlibc::QData configWhiteListDevices = whiteListResponse.getData("response").getData("info").getData("devices");
        size_t configWhiteListDevicesSize = configWhiteListDevices.size();

        qlibc::QData loadData;
        loadData.loadFromFile(R"(D:\project\byjs\exhibition\unit\bleSite\data\deviceList.json)");
        qlibc::QData deviceList = loadData.getData("device_list");
        size_t deviceListSize = deviceList.size();


        qlibc::QData newDeviceList;

        for (Json::ArrayIndex i = 0; i < deviceListSize; ++i) {
            qlibc::QData deviceItem = deviceList.getArrayElement(i);

            if (configWhiteListDevicesSize == 0) {
                qlibc::QData item;
                item.setString("device_id", deviceItem.getString("device_id"));
                newDeviceList.append(item);
                continue;
            }

            for (Json::ArrayIndex j = 0; j < configWhiteListDevicesSize; ++j) {
                qlibc::QData configDevice = configWhiteListDevices.getArrayElement(j);

                if (configDevice.getString("category_code") != "light") {
                    if (j == configWhiteListDevicesSize - 1) {
                        qlibc::QData item;
                        item.setString("device_id", deviceItem.getString("device_id"));
                        newDeviceList.append(item);
                    }
                    continue;
                }

                if (configDevice.getString("device_sn") == deviceItem.getString("device_id")) {
                    qlibc::QData location;
                    location.setString("room_name", configDevice.getString("room_name"));
                    location.setString("room_no", configDevice.getString("room_no"));

                    qlibc::QData item;
                    item.setString("device_id", configDevice.getString("device_sn"));
                    item.setString("device_name", configDevice.getString("device_name"));
                    item.setString("device_type", configDevice.getString("device_type"));
                    item.setString("device_brand", configDevice.getString("device_brand"));
                    item.putData("location", location);

                    newDeviceList.append(item);
                    break;
                }

                if (j == configWhiteListDevicesSize - 1) {
                    qlibc::QData item;
                    item.setString("device_id", deviceItem.getString("device_id"));
                    newDeviceList.append(item);
                }
            }
        }

        //存储设备列表
        qlibc::QData saveData;
        saveData.putData("device_list", newDeviceList);
        LOG_INFO << "saveData: " << saveData.toJsonString(true);
    }

}


int main(int argc, char* argv[]){

    httplib::Request request;
    updateDeviceList(request);


    return 0;
}
















