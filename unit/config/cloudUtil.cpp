//
// Created by 78472 on 2022/5/11.
//

#include "cloudUtil.h"
#include <utility>
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include "socket/httplib.h"
#include "myutil.h"


cloudUtil* cloudUtil::instance = nullptr;


cloudUtil::cloudUtil(){}

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
}

bool cloudUtil::joinTvWhite() {
    qlibc::QData recordData = configParamUtil::getInstance()->getRecordData();
    bool tvWhite = recordData.getBool("tvWhite");

    if(!tvWhite){
        //todo 获取tv信息
        qlibc::QData TvInfo;

        qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
        baseInfoData.setString("deviceSn", TvInfo.getString("tv_mac"));
        baseInfoData.setString("deviceMac", TvInfo.getString("tv_mac"));
        configParamUtil::getInstance()->saveBaseInfo(baseInfoData);

        qlibc::QData requestBody;
        requestBody.setString("productVender", "changhong");
        requestBody.setString("productType", "电视");
        requestBody.setString("productName", TvInfo.getString("tv_name"));
        requestBody.setString("productModel",TvInfo.getString("tv_model"));
        requestBody.setString("deviceSn",    TvInfo.getString("tv_mac"));
        requestBody.setString("secretMsg", lhytemp::myutil::getSecretMsg(dataDirPath));

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
            std::cout << "===>join tvWhite failed, retry again-----" << std::endl;
        }

        string tvDid = returnMessage.getData("data").getString("deviceDid");

        baseInfoData.setString("tvDid", tvDid);
        configParamUtil::getInstance()->saveBaseInfo(baseInfoData);
        recordData.setBool("tvWhite", true);
        configParamUtil::getInstance()->saveRecordData(recordData);
    }

    return true;
}


bool cloudUtil::tvRegister(qlibc::QData& engineerInfo, qlibc::QData& responseData) {
    qlibc::QData recordData = configParamUtil::getInstance()->getRecordData();
    bool tvWhite = recordData.getBool("tvWhite");

    if(!tvWhite){   //如果电视未加入大白名单
        responseData.setString("code", "201");
        responseData.setString("msg", "joinTvWhite not finished.....");
        return false;
    }

    qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
    baseInfoData.setString("engineID", engineerInfo.getString("engineID"));
    baseInfoData.setString("domainSign", engineerInfo.getString("domainSign"));
    baseInfoData.setString("domainID", engineerInfo.getString("domainID"));
    configParamUtil::getInstance()->saveBaseInfo(baseInfoData);

    string tvDid = baseInfoData.getString("tvDid");
    time_t seconds = time(nullptr);

    Json::Value paramData;
    paramData["engineID"] = engineerInfo.getString("engineID");
    paramData["domainID"] = engineerInfo.getString("domainSign");
    paramData["domainSign"] = engineerInfo.getString("domainID");
    paramData["tvDid"] = baseInfoData.getString("tvDid");
    paramData["tvTimeStamp"] = std::to_string(seconds);
    paramData["tvSign"] = lhytemp::myutil::getTvSign(tvDid, std::to_string(seconds), dataDirPath);

    qlibc::QData message2Handle;
    qlibc::QData returnMessage;
    message2Handle.setString("User-Agent", "curl");
    message2Handle.setValue("param", paramData);

    ecb_httppost("/logic-device/edge/tvRegister", message2Handle, returnMessage);
    if(returnMessage.getInt("code") != 200){    //如果注册不成功
        responseData.setInitData(returnMessage);
        return false;
    }

    recordData.setBool("tvRegister", true);
    configParamUtil::getInstance()->saveRecordData(recordData);

    responseData.setInitData(returnMessage);

    //订阅相关主题
    if(!engineerInfo.getString("domainID").empty()){
        std::vector<string> topicVec;
        string topic = "edge/" + engineerInfo.getString("domainID") + "/device/domainWhite";
        topicVec.emplace_back(topic);
        for(auto &elem : topicVec){
            //todo mqtt订阅相关主题
//            mMqttClient.subscribe(elem);
            lhytemp::myutil::storeTopic(elem);
        }
    }

    return true;
}

bool cloudUtil::ecb_httppost(const string &uri, const qlibc::QData &request, qlibc::QData &response) {
    httplib::Client client(serverIp, serverPort);

    httplib::Headers header;
    Json::Value &root = request.asValue();
    for (string h : root.getMemberNames()) {
        if (h.empty() || h == "param" || h == "uri" || h[0] == '@') continue;
        //读入header参数，可能是@c的Object对象
        auto v = request.getValue(h);
        if (v.isObject() || v.isNull() || v.isArray()) continue;
        string str;
        qlibc::QData::valueToJsonString(v, str);
        if (str.empty()) continue;
        header.insert(std::pair<string, string>(h, str));
    }

    string body = request.getData("param").toJsonString();
    const char *key = "123456asdfgh1234";
    string out;
    lhytemp::myutil::ecb_encrypt_withPadding(body, out, reinterpret_cast<const uint8_t *>(key));

    auto http_res = client.Post(uri.c_str(), header, out, "application/json");
    if (http_res != nullptr) {
        string decryptOut;
        lhytemp::myutil::ecb_decrypt_withPadding(http_res->body.c_str(), decryptOut,
                                                 reinterpret_cast<const uint8_t *>(key));
        response = qlibc::QData(decryptOut);
        std::cout << "===>httpResponse: " << response.toJsonString() << std::endl;
        return true;
    }
    return false;
}







