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
    //接收串口返回，解析返回数据，产生相应的事件
    static bool handlePackageString(string& subPackageString);

    static void disableUpload();

private:
    static void parse2Event(string& completePackageString);
};


#endif //EXHIBITION_RECVPACKAGEPARSE_H
