//
// Created by 78472 on 2022/5/11.
//

#include "cloudUtil.h"
#include "qlibc/FileUtils.h"
#include "secretUtils.h"
#include "common/httpUtil.h"
#include <thread>
#include <chrono>
#include "../param.h"
#include "log/Logging.h"


cloudUtil* cloudUtil::instance = nullptr;

cloudUtil *cloudUtil::getInstance() {
    if(instance == nullptr)
        instance = new cloudUtil;
    return instance;
}

void cloudUtil::destroyInstance() {
    if(instance != nullptr){
        delete instance;
        instance = nullptr;
    }
}

void cloudUtil::init(const string&  ip, int port, const string& dataDirectoryPath) {
    serverIp = ip;
    serverPort = port;
    dataDirPath = dataDirectoryPath;
    LOG_PURPLE << "cloudUtil init: httpServerIp<" << serverIp << ">---httpServerPort<" << serverPort
              << ">---dataDirPath<" << dataDirPath << ">";
}

bool cloudUtil::getTvInfo(string& tvMac, string& tvName, string& tvModel){
    qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
    tvMac = baseInfoData.getString("deviceMac");
    tvName = baseInfoData.getString("tv_name");
    tvModel = baseInfoData.getString("tv_model");

    if(tvMac.empty() || tvName.empty() || tvModel.empty()){
        qlibc::QData tvRequest, tvResponse;
        tvRequest.setString("service_id", "get_tv_mac");
        tvRequest.putData("request", qlibc::QData());

        auto ret = httpUtil::sitePostRequest(RequestIp, AdapterPort, tvRequest, tvResponse);
        if(ret){
            if(tvResponse.getInt("code") == 0){
                tvMac = tvResponse.getData("response").getString("tv_mac");
                tvName = tvResponse.getData("response").getString("tv_name");
                tvModel = tvResponse.getData("response").getString("tv_model");

                baseInfoData.setString("deviceSn", tvMac);
                baseInfoData.setString("deviceMac", tvMac);
                baseInfoData.setString("tv_name", tvName);
                baseInfoData.setString("tv_model", tvModel);
                configParamUtil::getInstance()->saveBaseInfo(baseInfoData);
                return true;
            }
            return false;
        }
        return false;
    }
    return true;
}


bool cloudUtil::joinTvWhite() {
    //??????????????????,??????????????????
    qlibc::QData recordData = configParamUtil::getInstance()->getRecordData();
    bool tvWhite = recordData.getBool("tvWhite");

    if(!tvWhite){
        //??????????????????
        string tvMac, tvName, tvModel;
        while(true){
            bool ret = getTvInfo(tvMac, tvName, tvModel);
            if(ret)
                break;
            else{
                LOG_RED << "---can no get tvInfo from adapter site, try again in 3 seconds......";
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }

        //???????????????????????????????????????
        qlibc::QData requestBody;
        requestBody.setString("productVender", "changhong");
        requestBody.setString("productType", "??????");
        requestBody.setString("productName", tvName);
        requestBody.setString("productModel", tvModel);
        requestBody.setString("deviceSn", tvMac);
        requestBody.setString("secretMsg", lhytemp::secretUtil::getSecretMsg(dataDirPath));

        qlibc::QData message2Handle;
        message2Handle.setString("User-Agent", "curl");
        message2Handle.setString("uri", "/logic-device/edge/tvWhite");
        message2Handle.setValue("param", requestBody.asValue());

        qlibc::QData returnMessage;
        while(true) {
            ecb_httppost("/logic-device/edge/tvWhite", message2Handle, returnMessage);
            if(returnMessage.getString("code") == "200")
                break;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            LOG_RED << "===>join tvWhite failed, retry again-----";
        }

        string tvDid = returnMessage.getData("data").getString("deviceDid");

        qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
        baseInfoData.setString("tvDid", tvDid);
        configParamUtil::getInstance()->saveBaseInfo(baseInfoData);

        recordData.setBool("tvWhite", true);
        configParamUtil::getInstance()->saveRecordData(recordData);
    }

    return true;
}


bool cloudUtil::tvRegister(mqttClient& mc, qlibc::QData& engineerInfo, qlibc::QData& responseData) {

    //??????????????????????????????????????? ????????????????????????????????????
    qlibc::QData recordData = configParamUtil::getInstance()->getRecordData();
    bool tvWhite = recordData.getBool("tvWhite");
    if(!tvWhite){   //?????????????????????????????????
        responseData.setString("code", "201");
        responseData.setString("msg", "joinTvWhite not finished.....");
        return false;
    }

    //?????????????????????????????????
    qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
    string tvDid = baseInfoData.getString("tvDid");
    time_t seconds = time(nullptr);

    //???????????????????????????
    Json::Value paramData;
    paramData["engineID"] = engineerInfo.getString("engineID");
    paramData["domainID"] = engineerInfo.getString("domainID");
    paramData["domainSign"] = engineerInfo.getString("domainSign");
    paramData["tvDid"] = baseInfoData.getString("tvDid");
    paramData["tvTimeStamp"] = std::to_string(seconds);
    paramData["tvSign"] = lhytemp::secretUtil::getTvSign(tvDid, std::to_string(seconds), dataDirPath);

    qlibc::QData message2Handle;
    qlibc::QData returnMessage;
    message2Handle.setString("User-Agent", "curl");
    message2Handle.setValue("param", paramData);

    ecb_httppost("/logic-device/edge/tvRegister", message2Handle, returnMessage);
    responseData.setInitData(returnMessage);
    if(returnMessage.getInt("code") != 200){    //?????????????????????
        return false;
    }

    //??????????????????????????????
    baseInfoData.setString("engineID", engineerInfo.getString("engineID"));
    baseInfoData.setString("domainSign", engineerInfo.getString("domainSign"));
    baseInfoData.setString("domainID", engineerInfo.getString("domainID"));
    configParamUtil::getInstance()->saveBaseInfo(baseInfoData);

    recordData.setBool("tvRegister", true);
    configParamUtil::getInstance()->saveRecordData(recordData);

   //?????????????????????
    string domainID = engineerInfo.getString("domainID");
    if(!domainID.empty()){
        string topic = "edge/" + domainID + "/device/domainWhite";
        mc.subscribe(topic, 0);
    }

    return true;
}

bool cloudUtil::ecb_httppost(const string &uri, const qlibc::QData &request, qlibc::QData &response) {
    httplib::Client client(serverIp, serverPort);

    httplib::Headers header;
    Json::Value &root = request.asValue();
    for (string h : root.getMemberNames()) {
        if (h.empty() || h == "param" || h == "uri" || h[0] == '@') continue;
        //??????header??????????????????@c???Object??????
        auto v = request.getValue(h);
        if (v.isObject() || v.isNull() || v.isArray()) continue;
        string str;
        qlibc::QData::valueToJsonString(v, str);
        if (str.empty()) continue;
        header.insert(std::pair<string, string>(h, str));
    }

    string body = request.getData("param").toJsonString();
    LOG_BLUE << "===>httpRequest: " << body;

    const char *key = "123456asdfgh1234";
    string out;
    lhytemp::secretUtil::ecb_encrypt_withPadding(body, out, reinterpret_cast<const uint8_t *>(key));

    auto http_res = client.Post(uri.c_str(), header, out, "application/json");
    if (http_res != nullptr) {
        string decryptOut;
        lhytemp::secretUtil::ecb_decrypt_withPadding(http_res->body.c_str(), decryptOut,
                                                 reinterpret_cast<const uint8_t *>(key));
        response = qlibc::QData(decryptOut);
        LOG_YELLOW << "===>httpResponse: " << response.toJsonString();
        return true;
    }
    return false;
}







