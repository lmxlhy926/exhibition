#ifndef STRIPLIGHT_LIGHTMANAGE_H
#define STRIPLIGHT_LIGHTMANAGE_H

#include "stripLight.h"
#include <map>
#include <vector>
#include <mutex>

class lightManage{
private:
    string start_time;  //响应起始时间
    string end_time;    //响应结束时间
    std::recursive_mutex Mutex;
    std::map<string, stripLight> stripLightContainer;   //灯带管理<deviceid, stripLight>
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
    bool addExecuteObj(qlibc::QData& data, qlibc::QData deviceList = qlibc::QData());

    //删除夜灯执行对象
    void delExecuteObj(qlibc::QData& data);

    //获取夜灯灯带列表
    qlibc::QData getLogicalStripList();

    //更新物理灯带信息
    void updatePhysicalStrip(qlibc::QData& deviceList);

     //处理雷达点位
    void handleRadarPoints(qlibc::QData& pointData);

private:
    //获取设备列表
    bool getDeviceList(qlibc::QData& deviceList);

    //获取白名单
    bool getWhiteList(qlibc::QData& whiteList);

     //格式化物理灯带参数
    StripParamType stripData2Struct(const Json::Value& data);

    //获取灯带物理参数
    StripParamType getStripParam(const string& device_id, qlibc::QData& deviceList);

    //逻辑灯带格式转换
    LogicalStripType logicalStripValue2Struct(Json::Value const& value);

    //从文件加载夜灯列表
    void loadStripLightsContainer();

    //存储灯带列表
    void storeStripLightsContainer();

    //判断时刻是否在响应范围内
    bool isInValidTime();

    //获取区域房间对应map
    std::map<string, string> getAreaRoomMap(qlibc::QData& data);

    //区域房间号转换
    string areaNum2RoomNo(const string& areaNo, std::map<string, string> const& areaRoomMap);

    //坐标点位转换
    std::vector<CoordPointType> getCoordPointVec(qlibc::QData& targetList);

    //转换坐标
    RadarPointsType trans2PointSequence(qlibc::QData& pointData);

    //打印转换坐标集
    void printPointSequence(const RadarPointsType& radarPoints);
};

#endif