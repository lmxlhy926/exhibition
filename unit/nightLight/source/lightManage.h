#ifndef STRIPLIGHT_LIGHTMANAGE_H
#define STRIPLIGHT_LIGHTMANAGE_H

#include "stripLight.h"
#include <map>
#include <vector>
#include <mutex>

class lightManage{
private:
    std::recursive_mutex Mutex;
    string start_time;  //响应起始时间
    string end_time;    //响应结束时间
    std::map<string, stripLight> stripLightContainer;   //灯带管理
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
    qlibc::QData getLogicalStripList();

    //处理雷达点位
    void handleRadarPoints(qlibc::QData& pointData);

private:
    //判断时刻是否在响应范围内
    bool isInValidTime();

    //获取设备列表
    qlibc::QData getDeviceList();

    //灯带结构参数转换
    StripParamType stripData2Struct(const Json::Value& data);

    //获取灯带物理参数
    StripParamType getStripParam(const string& device_id, qlibc::QData& deviceList);

    //获取白名单
    qlibc::QData getWhiteList();

    //获取区域房间对应map
    std::map<string, string> getAreaRoomMap(qlibc::QData& data);

    //区域房间号转换
    string areaNum2RoomNo(const string& areaNo, std::map<string, string> const& areaRoomMap);

    //坐标点位转换
    std::vector<CoordPointType> getCoordPointVec(qlibc::QData& targetList);

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