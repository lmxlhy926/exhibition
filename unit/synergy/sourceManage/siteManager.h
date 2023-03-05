//
// Created by 78472 on 2023/3/4.
//

#ifndef EXHIBITION_SITEMANAGER_H
#define EXHIBITION_SITEMANAGER_H

#include <thread>
#include "qlibc/QData.h"
using namespace std;

class siteManager {
public:
    //更新站点列表
    static void updateSiteBak();

    static qlibc::QData updateSite();
};


#endif //EXHIBITION_SITEMANAGER_H
