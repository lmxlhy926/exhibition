//
// Created by 78472 on 2022/6/13.
//

#ifndef EXHIBITION_RECVPACKAGEPARSE_H
#define EXHIBITION_RECVPACKAGEPARSE_H

#include <string>
#include <iostream>
#include <sstream>
#include <mutex>

using namespace std;

class RecvPackageParse{
    static mutex mutex_;
    static string packageString;
    static bool enable;
public:
    //读取串口产生的包数据，拼接包，解析完整包
    static bool handlePackageString(string& subPackageString);

    static void disableUpload();

private:
    static void parse2Event(string& completePackageString);
};


#endif //EXHIBITION_RECVPACKAGEPARSE_H
