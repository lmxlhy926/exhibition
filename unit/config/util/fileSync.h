//
// Created by 78472 on 2023/2/21.
//

#ifndef EXHIBITION_FILESYNC_H
#define EXHIBITION_FILESYNC_H

#include "http/httplib.h"

class fileSync {
public:
    static void subscribeOtherConfigSites();   //订阅其它配置站点

    static void publishSyncMessages();     //发布更新消息

    static void  updateFile(const httplib::Request& request);   //更新配置文件

private:
    static void updateConfigSites();   //更新配置站点
};

#endif //EXHIBITION_FILESYNC_H
