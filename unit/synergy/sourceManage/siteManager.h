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
    /*
     *  1. 获取所有站点列表，从中得到蓝牙、zigbee所在面板的IP
     *  2. 获得ip后，访问相应面板的配置站点，从中获取面板信息
     *  3. 删除不存在的站点
     *  4. 存储新的站点
     */
    static void updateSite();

    static void updateSite1();
};


#endif //EXHIBITION_SITEMANAGER_H
