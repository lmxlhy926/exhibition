//
// Created by 78472 on 2022/10/22.
//

#include "mqttHandle.h"
#include "qlibc/QData.h"
#include "log/Logging.h"
//#include "ssl/sslUtil.h"
#include "handle/mdmConfig.h"
#include "siteService/service_site_manager.h"
#include "../mdmParam.hpp"

bool mqttHandle::handle(const string &topic, char *payloadReceive, int len) {
    //接收并打印收到的mqtt消息
    qlibc::QData message(payloadReceive, len);
    if (message.type() == Json::nullValue) {
        LOG_RED << "received mqttPayload is not a Json format.......";
        return false;
    }
    LOG_INFO << "==>topic: " << topic << "----message: " << message.toJsonString();

    string domainID;
    if (topic == "edge/" + domainID + "/device/ysAuth") {     //授权信息
//        string authType = message.getString("authType");
//        if (authType == "authCode") {     //授权码信息
//            //获取用户授权信息
//            string auth_code = message.getString("authCode");
//            qlibc::QData requestMessage, res;
//            Json::Value param;
//            param["access_token"] = mdmConfig::getInstance()->getBaseInfoData().getString("accessToken");
//            param["auth_code"] = auth_code;
//            requestMessage.setValue("param", param);
//            sslUtil::getInstance()->sendRequest2Site(EZVIZ, "/api/lapp/trust/device/token/get", requestMessage, res);
//
//            //存储授权信息
//            qlibc::QData authInfoData;
//            authInfoData.putData("data", res.getData("data"));
//            authInfoData.setString("auth_code", auth_code);
//            mdmConfig::getInstance()->saveAuthInfoData(authInfoData);
//        }

    } else if (topic == "edge/" + domainID + "/device/ysCmd") {    //设备动作信息
        Json::ArrayIndex messageSize = message.size();
        for (Json::ArrayIndex i = 0; i < messageSize; ++i) {
            qlibc::QData item = message.getArrayElement(i);
            string type = item.getData("header").getString("type");
            if (type == "ys.calling" || type == "ys.transpipe.transmit") {
                //发布消息
                qlibc::QData messageData;
                messageData.setString("message_id", CALLING_MESSAGE_ID);
                messageData.putData("content", qlibc::QData());
                servicesite::ServiceSiteManager::getInstance()->publishMessage(CALLING_MESSAGE_ID, messageData.toJsonString());
                break;
            }
        }
    }

    return true;
}
