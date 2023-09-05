#ifndef STRIPLIGHT_LIGHTMANAGE_H
#define STRIPLIGHT_LIGHTMANAGE_H

#include "stripLight.h"
#include <map>
#include <vector>
#include <mutex>

class lightManage{
private:
    std::recursive_mutex Mutex;
    string start_time;
    string end_time;
    std::map<string, stripLight> stripLightContainer;
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

    //保存夜灯执行对象
    void addExecuteObj(qlibc::QData& data);

    //删除夜灯执行对象
    void delExecuteObj(qlibc::QData& data);

    //获取夜灯灯带列表
    qlibc::QData getLogiclStripList();

    //处理雷达点位
    void handleRadarPoints(qlibc::QData& pointData);

private:
    //获取设备列表
    qlibc::QData getDeviceList();

    //获取灯带物理参数
    StripParamType getStripParam(const string& device_id, qlibc::QData& deviceList);

    //转换坐标
    RadarPointsType trans2PointSequence(qlibc::QData& pointData);

    //从文件加载夜灯列表
    void loadStripLightsContainer();

    //存储灯带列表
    void storeStripLightsContainer();

    //格式转换
    LogicalStripType logicalStripValue2Struct(Json::Value const& value);
};

#endif