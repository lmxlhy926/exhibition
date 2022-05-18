//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_CLOUDUTIL_H
#define EXHIBITION_CLOUDUTIL_H

#include <string>
#include <mutex>
#include "qlibc/QData.h"
#include "socket/httplib.h"
#include "common/configParamUtil.h"
#include "mqtt/mqttClient.h"

using namespace std;


class cloudUtil {
private:
    string serverIp;
    int serverPort;
    string dataDirPath;
    static cloudUtil* instance;

private:
    explicit cloudUtil();

public:
    static cloudUtil* getInstance();

    static void destroyInstance();

    void init(const string&  ip, int port, const string& dataDirectoryPath);

    static bool getTvInfo(string& tvMac, string& tvName, string& tvModel);

    static bool getTvInfo(qlibc::QData& tvInfo);

    //阻塞，直到成功为止
    bool joinTvWhite();

    /**
     * 如果电视已经加入大白名单，则进行电视注册
     * 如果电视未加入大白名单，这直接返回。
     * @param engineerInfo 安装师傅信息
     * @return
     */
    bool tvRegister(mqttClient& mc, qlibc::QData& engineerInfo, qlibc::QData& responseData);

    //向云端发送http请求
    bool ecb_httppost(const std::string& uri, const qlibc::QData &request, qlibc::QData &response);
};


#endif //EXHIBITION_CLOUDUTIL_H
