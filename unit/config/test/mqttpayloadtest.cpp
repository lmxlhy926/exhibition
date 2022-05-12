

#include <string>
#include <algorithm>
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include "../configParamUtil.h"


using namespace std;


bool mqttPayloadHandle(const string& topic, qlibc::QData& mqttPayload){


    qlibc::QData baseinfo = configParamUtil::getInstance()->getBaseInfo();
    string domainID = baseinfo.getString("domainID");

    if(topic == "edge/" + domainID + "/device/domainWhite"){

        qlibc::QData payload(mqttPayload);


        qlibc::QDataList devices;
        payload.getData("info").getDataList("devices", devices);
        int devicesItemCount = devices.getItemCount();

        qlibc::QDataList lighDataList;
        qlibc::QDataList radarDataList;

        for (int i = 0; i < devicesItemCount; i++) {
            qlibc::QData ithData;
            devices.getItem(i, ithData);
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

            string productUsed = ithData.getString("productUsed");

            std::vector<string> keyNames{"categoryCode","device_id","deviceVender","deviceDid","productNickname","deviceDesc","productId",
                                         "productType","deviceType","productModel","deviceSn","deviceMac","familyCode","roomType","roomNo",
                                         "x","y","hubMac","installOrientation","radius","radiationAngle","rotateAngle","productNickname",
                                         "hubNum","coverArea"};
            Json::Value::Members allKeyNames = ithData.asValue().getMemberNames();

            for(auto pos = allKeyNames.begin(); pos != allKeyNames.end(); ++pos){
                auto findPos = find(keyNames.begin(), keyNames.end(), *pos);
                if(findPos == keyNames.end()){  //不存在于已转换的key中
                    dataObject.setValue(*pos, ithData.getValue(*pos));
                }
            }

            string device_type = ithData.getString("deviceType");
            std::vector<string> typeVector({"1", "2", "3", "4", "5", "6"});
            for(int i = 0; i < typeVector.size(); i++){
                string type= typeVector[i];
                if(device_type == type){
                    radarDataList.appendItem(dataObject);
                    break;
                }
            }

            string categoryCode = ithData.getString("categoryCode");
            if(categoryCode == "light"){
                dataObject.setString("device_type", productUsed);
                lighDataList.appendItem(dataObject);
            }
        }

        //传递白名单给ipvdal
        payload.asValue()["info"]["devices"] = radarDataList.asValue();
        string timeStr = std::to_string(time(nullptr));
        payload.setString("timeStamp", timeStr);

        qlibc::QData data2wlist;
        data2wlist.setValue("info", lighDataList.asValue());
    }

    return true;
}



int main(int argc, char* argv[]){



    return 0;
}
























