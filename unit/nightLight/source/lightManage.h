#ifndef STRIPLIGHT_LIGHTMANAGE_H
#define STRIPLIGHT_LIGHTMANAGE_H

#include "stripLight.h"
#include <map>
#include <mutex>

using stripLightConType = std::map<string, stripLight>;    //

class lightManage{
private:
    std::recursive_mutex Mutex;
    stripLightConType lightsContainer;
    static lightManage* Instance;
    
public:
    static lightManage* getInstance();

    //刷新夜灯列表
    void flushLightsContaienr(Json::Value& deviceLists);

    //收到雷达站点的点位消息后，调用此函数
    void calculateCoordPointAndControl(const string& pointsMessage);
};

#endif