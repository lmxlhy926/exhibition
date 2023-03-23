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
     *  1. 获取所有站点列表，从中得到蓝牙、zigbee所在面板的IP
     *  2. 获得ip后，访问相应面板的配置站点，从中获取面板信息
     *  3. 删除不存在的站点
     *  4. 存储新的站点
     */
    static void updateSite();

    //获取设备类站点
    static qlibc::QData getPanelList();
};


#endif //EXHIBITION_SITEMANAGER_H
