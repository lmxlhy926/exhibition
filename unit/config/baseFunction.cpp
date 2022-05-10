//
// Created by 78472 on 2022/5/10.
//

#include "baseFunction.h"
#include "myutil.h"

bool ecb_httppost(httplib::Client *client, const std::string& uri,
                  const qlibc::QData &request, qlibc::QData &response){

    if (client == nullptr) return false;

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

//    MGLogReq("Http ==> : %s --- %s", uri.c_str(), request.toJSONString().c_str());

    string body = request.getData("param").toJsonString();
    const char *key = "123456asdfgh1234";
    string out;
    lhytemp::myutil::ecb_encrypt_withPadding(body, out, reinterpret_cast<const uint8_t *>(key));

//    QLog("==>encrypt body: %s\n", out.c_str());

    auto http_res = client->Post(uri.c_str(), header, out, "application/json");
//    QLog("====>post successfully==\n");

    if(http_res == nullptr){
//        QLog("========http_res in nullptr========>\n");
    }

    if (http_res != nullptr) {
        string decryptOut;
        lhytemp::myutil::ecb_decrypt_withPadding(http_res->body.c_str(), decryptOut,
                                                 reinterpret_cast<const uint8_t *>(key));
        response = qlibc::QData(decryptOut);
//        MGLogRsp("Http <== : %s --- %s", uri.c_str(), response.toJSONString().c_str());
//        QLog("Http <== : %s --- %s\n", uri.c_str(), response.toJSONString().c_str());

        return true;
    }
    return false;
}


bool joinTvWhite(const string& cacheDir){

    qlibc::QData baseinfo;
    lhytemp::fileRW::loadFromFile(baseinfo, FileUtils::contactFileName(cacheDir, "baseinfo.json"));
    qlibc::QData record;
    lhytemp::fileRW::loadFromFile(record, FileUtils::contactFileName(cacheDir, "record.json"));
    bool tvWhite = record.getBool("tvWhite");

    if(!tvWhite){   //如果没加入大白名单
        qlibc::QData requestBody;
        qlibc::QData TvInfo;
        qlibc::QData requestData;
        requestData.setString("device_id", "tv_gateway_local");
        postServiceRequest("coss", "/system/getTVInfo", requestData, TvInfo);

        int tvInfoCode = TvInfo.getInt("code");
        while(tvInfoCode != 200){
            postServiceRequest("coss", "/system/getTVInfo", requestData, TvInfo);
            tvInfoCode = TvInfo.getInt("code");
        }

        requestBody.setString("productVender", "changhong");
        requestBody.setString("productType", "电视");
        requestBody.setString("productName", TvInfo.getData("payload").getString("tv_name"));
        requestBody.setString("productModel",TvInfo.getData("payload").getString("tv_model"));
        requestBody.setString("deviceSn",    TvInfo.getData("payload").getString("tv_mac"));

        httplib::Client client__(url, port);

        string sn = requestBody.getString("deviceSn");
        string mac = requestBody.getString("deviceMac");
        baseinfo.setString("deviceSn", sn);
        baseinfo.setString("deviceMac", mac);
        lhytemp::fileRW::saveToFile(baseinfo, FileUtils::contactFileName(cacheDir, "baseinfo.json"));

        string secretMsg = lhytemp::myutil::getSecretMsg(cacheDir, FileUtils::contactFileName(cacheDir, "baseinfo.json"));
        requestBody.setString("secretMsg", secretMsg);

        qlibc::QData message2Handle;
        message2Handle.setString("User-Agent", "curl");
        message2Handle.setString("uri", "/logic-device/edge/tvWhite");
        message2Handle.setValue("param", requestBody.asValue());

        qlibc::QData returnMessage;
        while(true) {
            ecb_httppost(&client__, "/logic-device/edge/tvWhite", message2Handle, returnMessage);
            if(returnMessage.getString("code") == "200")
                break;
            std::this_thread::sleep_for(std::chrono::seconds(2));
            QLog("===>join tvWhite failed, retry again-----\n");
        }

        string tvDid = returnMessage.getData("data").getString("deviceDid");
        baseinfo.setString("tvDid", tvDid);
        lhytemp::fileRW::saveToFile(baseinfo, FileUtils::contactFileName(cacheDir, "baseinfo.json"));

        record.setBool("tvWhite", true);
        lhytemp::fileRW::saveToFile(record, FileUtils::contactFileName(cacheDir, "record.json"));
    }

    //成功，或者已经注册后都会通知电视注册程序
    {
        std::unique_lock<std::mutex> ul(lhytemp::concurrency::registerMutex);
        lhytemp::concurrency::registerFlag = true;
    }
    lhytemp::concurrency::registerConVar.notify_all();

    return true;

}






































