//
// Created by 78472 on 2023/3/4.
//

#ifndef EXHIBITION_SITEMANAGER_H
#define EXHIBITION_SITEMANAGER_H

#include <thread>
#include "qlibc/QData.h"
using namespace std;

class siteManager {
private:
    static siteManager* Instance;

    //自动更新信息
    siteManager(){
        updateSite();
        new thread([]{
            while(true){
                std::this_thread::sleep_for(std::chrono::seconds(10));
                updateSite();
            }
        });
    }
public:
    static siteManager* getInstance();

    /*
        1. 更新siteRecord
        2. 更新设备列表，跟新组列表
     */
    static void updateSite();

    //获取设备类站点
    static qlibc::QData getPanelList();
};


#endif //EXHIBITION_SITEMANAGER_H
