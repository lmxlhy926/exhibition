//
// Created by 78472 on 2023/3/6.
//

#ifndef EXHIBITION_UTIL_H
#define EXHIBITION_UTIL_H

#include <string>
#include <mutex>
using namespace std;

class util {
private:
    static string panelId;
    static std::recursive_mutex rMutex;
public:
    //通知设备管理站点进行设备列表更新
    static void updateDeviceList();

    //获取面板信息
    static string getSourceSite();

    //修改
    static void modifyPanelProperty();
};


#endif //EXHIBITION_UTIL_H
