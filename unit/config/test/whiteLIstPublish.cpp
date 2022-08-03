
#include <string>
#include <iostream>
#include <algorithm>
#include <thread>
#include <chrono>
#include <fstream>
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include "mqtt/mqttClient.h"
#include "../util/secretUtils.h"

using namespace std;
using namespace qlibc;


/*
 * 转换白名单测试
 */
bool payloadTest(){
    qlibc::QData payload;
    payload.loadFromFile(R"(D:\bywg\project\exhibition\test\origin.json)");

    qlibc::QData devices;
    payload.getData("info").getData("devices", devices);
    Json::ArrayIndex devicesItemCount = devices.size();

    qlibc::QData deviceDataList;


    for (int i = 0; i < devicesItemCount; i++) {
        qlibc::QData ithData;
        devices.getArrayElement(i, ithData);

        qlibc::QData dataObject;
        dataObject.setString("category_code", ithData.getString("categoryCode"));
        long device_id;
        try{
            device_id = stol(ithData.getString("deviceId"), nullptr, 10);
            device_id %= 65536;
        }catch(const exception& e){
            device_id = 0;
        }

        dataObject.setInt("device_id", static_cast<int>(device_id));     //long
        dataObject.setString("device_brand", ithData.getString("deviceVender"));
        dataObject.setString("device_did", ithData.getString("deviceDid"));
        dataObject.setString("device_name", ithData.getString("productNickname"));
        dataObject.setString("device_desc", ithData.getString("deviceDesc"));
        dataObject.setInt("product_id", ithData.getInt("productId"));               //int
        dataObject.setString("product_type", ithData.getString("productType"));
        dataObject.setString("device_type", ithData.getString("deviceType"));
        dataObject.setString("product_model", ithData.getString("productModel"));
        dataObject.setString("device_sn", ithData.getString("deviceSn"));
        dataObject.setString("device_mac", ithData.getString("deviceMac"));
        dataObject.setString("family_code", ithData.getString("familyCode"));
        dataObject.setString("room_name", ithData.getString("roomType"));
        dataObject.setString("room_no", ithData.getString("roomNo"));
        dataObject.setString("x", ithData.getString("x"));
        dataObject.setString("y", ithData.getString("y"));
        dataObject.setString("hub_mac", ithData.getString("hubMac"));
        dataObject.setString("install_orientation", ithData.getString("installOrientation"));
        dataObject.setInt("radius", ithData.getInt("radius"));                      //int
        dataObject.setInt("radiation_angle", ithData.getInt("radiationAngle"));     //int
        dataObject.setInt("rotate_angle", ithData.getInt("rotateAngle"));           //int
        dataObject.setString("nick_name", ithData.getString("productNickname"));
        dataObject.setString("hub_num", ithData.getString("hubNum"));
        dataObject.setString("coverArea", ithData.getString("coverArea"));

        string categoryCode = ithData.getString("categoryCode");
        if(categoryCode == "light"){
            dataObject.setString("device_type", ithData.getString("productUsed"));
            //lighDataList.append(dataObject);
        }


        std::vector<string> keyNames{"categoryCode","device_id","deviceVender","deviceDid","productNickname","deviceDesc","productId",
                                     "productType","deviceType","productModel","deviceSn","deviceMac","familyCode","roomType","roomNo",
                                     "x","y","hubMac","installOrientation","radius","radiationAngle","rotateAngle","productNickname",
                                     "hubNum","coverArea"};
        Json::Value::Members allKeyNames = ithData.getMemberNames();

        for(auto pos = allKeyNames.begin(); pos != allKeyNames.end(); ++pos){
            auto findPos = find(keyNames.begin(), keyNames.end(), *pos);
            if(findPos == keyNames.end()){  //不存在于已转换的key中
                dataObject.setValue(*pos, ithData.getValue(*pos));
            }
        }

        deviceDataList.append(dataObject);
    }

    //传递白名单给ipvdal
    payload.asValue()["info"]["devices"] = deviceDataList.asValue();
    string timeStr = std::to_string(time(nullptr));
    payload.setString("timeStamp", timeStr);

    payload.saveToFile(R"(D:\bywg\project\exhibition\test\out.json)", true);


    return true;
}


/*
 *
 */
void publish(){
    string dataFileName = R"(./whiteList.json)";
    string configFileName = R"(./mqttConfig.json)";

    QData data, configData;
    data.loadFromFile(dataFileName);
    configData.loadFromFile(configFileName);

    std::string server = configData.getString("server");
    int port = configData.getInt("port");
    std::string username = configData.getString("username");
    std::string password = configData.getString("password");

    mqttClient mc;
    mc.paramConfig(server, port, username, password, "ssssss");
    mc.connect();

    string body = data.toJsonString();
    const char *key = "123456asdfgh1234";
    string out;
    lhytemp::secretUtil::ecb_encrypt_withPadding(body, out, reinterpret_cast<const uint8_t *>(key));

    while(true){
        mc.publish("edge/did:chisid:0xd7b8e6f93525720551baabdf56ecc3035492860d/device/domainWhite", out, 0);
        std::this_thread::sleep_for(std::chrono::seconds(10));
    }

}




int main(int argc, char* argv[]){

    publish();

    while(true){
        std::this_thread::sleep_for(std::chrono::seconds(20));
    }

    return 0;
}
























