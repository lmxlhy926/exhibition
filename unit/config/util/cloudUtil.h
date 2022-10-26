//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_CLOUDUTIL_H
#define EXHIBITION_CLOUDUTIL_H

#include <string>
#include <mutex>
#include "qlibc/QData.h"
#include "http/httplib.h"
#include "configParamUtil.h"
#include "mqtt/mqttClient.h"

using namespace std;


/**
 * 和云端http服务器进行交互的单例模式类，提供以下操作：
 *      1. 初始化服务器信息
 *      2. 向云端发送http请求
 *      2. 获取tv信息
 *      3. 加入大白名单
 *      4. 电视注册
 */
class cloudUtil {
private:
    string serverIp;                                //云端http服务器Ip
    int serverPort;                                 //云端http服务器端口号
    string dataDirPath;                             //配置文件路径
    std::unique_ptr<httplib::Client> client_;       //连接云端客户端
    static cloudUtil* instance;

private:
    explicit cloudUtil() = default;

public:
    static cloudUtil* getInstance();

    static void destroyInstance();

    //初始化http服务器配置参数
    void init(const string&  ip, int port, const string& dataDirectoryPath);

    //从Adapter站点获取tvMac
    static bool getTvInfo(string& tvMac, string& tvName, string& tvModel);

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
