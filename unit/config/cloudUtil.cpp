//
// Created by 78472 on 2022/5/11.
//

#include "cloudUtil.h"
#include <utility>
#include "qlibc/QData.h"
#include "qlibc/FileUtils.h"
#include "socket/httplib.h"
#include "myutil.h"

cloudUtil::cloudUtil(string  ip, int port,
                       string  dataDirectoryPath) :
                       serverIp(std::move(ip)), serverPort(port), dataDirPath(std::move(dataDirectoryPath)){}


bool cloudUtil::joinTvWhite() {
    qlibc::QData baseinfo;
    baseinfo.loadFromFile(FileUtils::contactFileName(dataDirPath, "baseinfo.json"));
    qlibc::QData record;
    record.loadFromFile(FileUtils::contactFileName(dataDirPath, "record.json"));
    bool tvWhite = record.getBool("tvWhite");

    if(!tvWhite){   //如果没加入大白名单
//        qlibc::QData requestData;
//        requestData.setString("device_id", "tv_gateway_local");
//        postServiceRequest("coss", "/system/getTVInfo", requestData, TvInfo);
//
//        int tvInfoCode = TvInfo.getInt("code");
//        while(tvInfoCode != 200){
//            postServiceRequest("coss", "/system/getTVInfo", requestData, TvInfo);
//            tvInfoCode = TvInfo.getInt("code");
//        }

        //todo 获取tv信息
        qlibc::QData TvInfo;

        qlibc::QData requestBody;
        requestBody.setString("productVender", "changhong");
        requestBody.setString("productType", "电视");
        requestBody.setString("productName", TvInfo.getData("payload").getString("tv_name"));
        requestBody.setString("productModel",TvInfo.getData("payload").getString("tv_model"));
        requestBody.setString("deviceSn",    TvInfo.getData("payload").getString("tv_mac"));

        httplib::Client client(serverIp, serverPort);

        string sn = requestBody.getString("deviceSn");
        string mac = requestBody.getString("deviceMac");
        baseinfo.setString("deviceSn", sn);
        baseinfo.setString("deviceMac", mac);
        baseinfo.saveToFile(FileUtils::contactFileName(dataDirPath, "baseinfo.json"));

        string secretMsg = lhytemp::myutil::getSecretMsg(dataDirPath, FileUtils::contactFileName(dataDirPath, "baseinfo.json"));
        requestBody.setString("secretMsg", secretMsg);

        qlibc::QData message2Handle;
        message2Handle.setString("User-Agent", "curl");
        message2Handle.setString("uri", "/logic-device/edge/tvWhite");
        message2Handle.setValue("param", requestBody.asValue());

        qlibc::QData returnMessage;
        while(true) {
            ecb_httppost(client, "/logic-device/edge/tvWhite", message2Handle, returnMessage);
            if(returnMessage.getString("code") == "200")
                break;
            std::this_thread::sleep_for(std::chrono::seconds(2));
//            QLog("===>join tvWhite failed, retry again-----\n");
        }

        string tvDid = returnMessage.getData("data").getString("deviceDid");
        baseinfo.setString("tvDid", tvDid);
        baseinfo.saveToFile(FileUtils::contactFileName(dataDirPath, "baseinfo.json"));

        record.setBool("tvWhite", true);
        record.saveToFile(FileUtils::contactFileName(dataDirPath, "record.json"));
    }

    //成功，或者已经注册后都会通知电视注册程序
    {
        std::unique_lock<std::mutex> ul(lhytemp::concurrency::registerMutex);
        lhytemp::concurrency::registerFlag = true;
    }
    lhytemp::concurrency::registerConVar.notify_all();

    return true;
}

bool cloudUtil::tvRegister(qlibc::QData& engineerInfo) {
    qlibc::QData record;
    record.loadFromFile(FileUtils::contactFileName(dataDirPath, "record.json"));

    bool tvWhite = record.getBool("tvWhite");

    if(!tvWhite){   //如果电视未加入大白名单
        std::unique_lock<std::mutex> ul(lhytemp::concurrency::registerMutex);
        lhytemp::concurrency::registerConVar.wait(ul, []{return lhytemp::concurrency::registerFlag;});
        lhytemp::concurrency::registerFlag = false;
    }

    string engineID = engineerInfo.getData("param").getString("engineID");
    string domainSign = engineerInfo.getData("param").getString("domainSign");
    string domainID = engineerInfo.getData("param").getString("domainID");

    qlibc::QData baseinfo;
    baseinfo.loadFromFile(FileUtils::contactFileName(dataDirPath, "baseinfo.json"));
    baseinfo.setString("engineID", engineID);
    baseinfo.setString("domainSign", domainSign);
    baseinfo.setString("domainID", domainID);
    baseinfo.saveToFile(FileUtils::contactFileName(dataDirPath, "baseinfo.json"));

    //边缘电视注册
    httplib::Client client(serverIp, serverPort);

    time_t seconds;
    seconds = time(nullptr);
    string tvDid = baseinfo.getString("tvDid");
    string tvSign = lhytemp::myutil::getTvSign(tvDid, std::to_string(seconds), dataDirPath);

    Json::Value paramData;
    paramData["engineID"] = engineID;
    paramData["domainID"] = domainID;
    paramData["domainSign"] = domainSign;
    paramData["tvDid"] = baseinfo.getString("tvDid");
    paramData["tvTimeStamp"] = std::to_string(seconds);
    paramData["tvSign"] = tvSign;

    qlibc::QData message2Handle;
    qlibc::QData returnMessage;
    message2Handle.setString("User-Agent", "curl");
    message2Handle.setValue("param", paramData);

    ecb_httppost(client, "/logic-device/edge/tvRegister", message2Handle, returnMessage);
    if(returnMessage.getInt("code") != 200){    //如果注册不成功
        return true;
    }

    record.setBool("tvRegister", true);
    record.saveToFile(FileUtils::contactFileName(dataDirPath, "record.json"));

    //订阅相关主题
    if(!domainID.empty()){
        std::vector<string> topicVec;
        string topic = "edge/" + domainID + "/device/domainWhite";
        topicVec.emplace_back(topic);
        for(auto &elem : topicVec){
            //todo mqtt订阅相关主题
//            mMqttClient.subscribe(elem);
            lhytemp::myutil::storeTopic(elem);
        }
    }

    return true;

}

bool cloudUtil::ecb_httppost(httplib::Client &client, const string &uri, const qlibc::QData &request,
                             qlibc::QData &response) {
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
