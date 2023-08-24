#ifndef STRIPLIGHT_LIGHTMANAGE_H
#define STRIPLIGHT_LIGHTMANAGE_H

#include "stripLight.h"
#include <map>
#include <vector>
#include <mutex>

class lightManage{
private:
    std::recursive_mutex Mutex;
    std::vector<stripLight> lightsContainer;
    static lightManage* Instance;
    
public:
    static lightManage* getInstance();

    //刷新夜灯列表
    void flushLightsContaienr(Json::Value& deviceLists);

    //收到雷达站点的点位消息后，调用此函数
    void calculateCoordPointAndControl(const string& pointsMessage);

private:
    //转换坐标
    PointSequenceType trans2PointSequence(const string& pointMessage);
};

#endif