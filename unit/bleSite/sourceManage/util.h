//
// Created by 78472 on 2023/3/6.
//

#ifndef EXHIBITION_UTIL_H
#define EXHIBITION_UTIL_H

#include <string>
#include <mutex>
#include "http/httplib.h"
using namespace httplib;
using namespace std;

class util {
private:
    static string panelId;
    static std::recursive_mutex rMutex;
public:
    //通知设备管理站点进行设备列表更新
    static void updateDeviceList();

    //通知设备管理站点对组列表进行更新
    static void updateGroupList();

    //获取面板信息
    static string getSourceSite();

    //面板属性被修改
    static void modifyPanelProperty(const Request& request);
};


#endif //EXHIBITION_UTIL_H
