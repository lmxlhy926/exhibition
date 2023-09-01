#ifndef STRIPLIGHT_LIGHTMANAGE_H
#define STRIPLIGHT_LIGHTMANAGE_H

#include "stripLight.h"
#include <map>
#include <vector>
#include <mutex>

class lightManage{
private:
    std::recursive_mutex Mutex;
    std::vector<stripLight> stripLightContainer;
    static lightManage* Instance;
    lightManage(){
        loadStripLightsContainer();
    }
public:
    static lightManage* getInstance(){
        if(nullptr == Instance){
            Instance = new lightManage();
        }
        return Instance;
    }

    //保存夜灯灯带
    void addLogicalStrip(qlibc::QData& data);

    //删除夜灯灯带
    void delLogicalStrip(qlibc::QData& data);

    //获取夜灯灯带列表
    qlibc::QData getLogicalStripList();

    //处理雷达点位
    void handleRadarPoints(qlibc::QData& pointData);

private:
    //转换坐标
    RadarPointsType trans2PointSequence(qlibc::QData& pointData);

    //从文件加载夜灯列表
    void loadStripLightsContainer();

    //存储灯带列表
    void storeStripLightsContainer();
};

#endif