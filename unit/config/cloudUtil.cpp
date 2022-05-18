//
// Created by 78472 on 2022/5/11.
//

#include "cloudUtil.h"
#include "qlibc/FileUtils.h"
#include "myutil.h"
#include "common/httpUtil.h"
#include <thread>
#include <chrono>


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
    std::cout << "cloudUtil init: serverIp<" << serverIp << ">---serverPort<" << serverPort
              << ">---dataDirPath<" << dataDirPath << ">" << std::endl;
}

bool cloudUtil::joinTvWhite() {
    //加载记录信息,加载基本信息
    qlibc::QData recordData = configParamUtil::getInstance()->getRecordData();
    qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
    bool tvWhite = recordData.getBool("tvWhite");

    if(!tvWhite){
        string tvMac = baseInfoData.getString("deviceMac");
        string tvName = baseInfoData.getString("tv_name");
        string tvModel = baseInfoData.getString("tv_model");

        if(tvMac.empty() || tvName.empty() || tvModel.empty()){
            qlibc::QData tvRequest, tvResponse;
            tvRequest.setString("service_id", "get_tv_mac");
            tvRequest.putData("request", qlibc::QData());

            while(true){
                auto ret = httpUtil::sitePostRequest("127.0.0.1", ADAPTER_PORT, tvRequest, tvResponse);
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
                        break;
                    }
                }
                std::cout << "---can no get tvInfo from adapter site, try again in 3 seconds......" << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(3));
            }
        }

        //构造请求体，上传生成的秘钥
        qlibc::QData requestBody;
        requestBody.setString("productVender", "changhong");
        requestBody.setString("productType", "电视");
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


bool cloudUtil::tvRegister(mqttClient& mc, qlibc::QData& engineerInfo, qlibc::QData& responseData) {

    //判断电视是否加入大白名单， 如果没有加入，则直接返回
    qlibc::QData recordData = configParamUtil::getInstance()->getRecordData();
    bool tvWhite = recordData.getBool("tvWhite");
    if(!tvWhite){   //如果电视未加入大白名单
        responseData.setString("code", "1");
        responseData.setString("error", "joinTvWhite not finished.....");
        responseData.putData("response", qlibc::QData());
        return false;
    }

    //将安装师傅信息存入文件
    qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();

    string tvDid = baseInfoData.getString("tvDid");
    time_t seconds = time(nullptr);

    //构造电视注册请求体
    Json::Value paramData;
    paramData["engineID"] = engineerInfo.getString("engineID");
    paramData["domainID"] = engineerInfo.getString("domainSign");
    paramData["domainSign"] = engineerInfo.getString("domainID");
    paramData["tvDid"] = baseInfoData.getString("tvDid");
    paramData["tvTimeStamp"] = std::to_string(seconds);
    paramData["tvSign"] = lhytemp::secretUtil::getTvSign(tvDid, std::to_string(seconds), dataDirPath);

    qlibc::QData message2Handle;
    qlibc::QData returnMessage;
    message2Handle.setString("User-Agent", "curl");
    message2Handle.setValue("param", paramData);

    ecb_httppost("/logic-device/edge/tvRegister", message2Handle, returnMessage);
    if(returnMessage.getInt("code") != 200){    //如果注册不成功
        responseData.setInitData(returnMessage);
        return false;
    }

    //注册成功则保存信息到
    baseInfoData.setString("engineID", engineerInfo.getString("engineID"));
    baseInfoData.setString("domainSign", engineerInfo.getString("domainSign"));
    baseInfoData.setString("domainID", engineerInfo.getString("domainID"));
    configParamUtil::getInstance()->saveBaseInfo(baseInfoData);

    recordData.setBool("tvRegister", true);
    configParamUtil::getInstance()->saveRecordData(recordData);

    responseData.setInitData(returnMessage);

   //订阅相关的主题
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
    lhytemp::secretUtil::ecb_encrypt_withPadding(body, out, reinterpret_cast<const uint8_t *>(key));

    auto http_res = client.Post(uri.c_str(), header, out, "application/json");
    if (http_res != nullptr) {
        string decryptOut;
        lhytemp::secretUtil::ecb_decrypt_withPadding(http_res->body.c_str(), decryptOut,
                                                 reinterpret_cast<const uint8_t *>(key));
        response = qlibc::QData(decryptOut);
        std::cout << "===>httpResponse: " << response.toJsonString() << std::endl;
        return true;
    }
    return false;
}







