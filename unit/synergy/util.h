//
// Created by 78472 on 2022/5/18.
//

#ifndef EXHIBITION_UTIL_H
#define EXHIBITION_UTIL_H

#include <string>
#include "qlibc/QData.h"
#include "common/configParamUtil.h"
#include "common/httpUtil.h"

using namespace std;

class util {
public:
    static bool getTvInfo(string& tvMac, string& tvName, string& tvModel);

    static bool getTvInfo(qlibc::QData& tvInfo);
};


#endif //EXHIBITION_UTIL_H
