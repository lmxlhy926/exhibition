//
// Created by 78472 on 2022/5/11.
//

#ifndef EXHIBITION_CLOUDUTIL_H
#define EXHIBITION_CLOUDUTIL_H

#include <string>
#include <mutex>
#include "qlibc/QData.h"
#include "socket/httplib.h"

using namespace std;

class cloudUtil {
private:
    const string serverIp;
    const int serverPort;
    const string dataDirPath;
    mutex registerMutex;
    condition_variable registerConVar;
    bool registerFlag;

public:
    explicit cloudUtil(string  ip, int port, string  dataDirectoryPath);

    //阻塞，直到成功为止
    bool joinTvWhite();

    /**
     * 如果电视已经加入大白名单，则进行电视注册
     * 如果电视未加入大白名单，这直接返回。
     * @param engineerInfo 安装师傅信息
     * @return
     */
    bool tvRegister(qlibc::QData& engineerInfo);

    //向云端发送http请求
    bool ecb_httppost(httplib::Client& client, const std::string& uri,
                      const qlibc::QData &request, qlibc::QData &response);
};


#endif //EXHIBITION_CLOUDUTIL_H
