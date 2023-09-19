#ifndef STRIPLIGHT_LIGHTMANAGE_H
#define STRIPLIGHT_LIGHTMANAGE_H

#include "stripLight.h"
#include <map>
#include <vector>
#include <mutex>
#include <sys/time.h>
#include "sendBuffer.h"
#include "common/httpUtil.h"

class lightManage{
private:
    string start_time;  //响应起始时间
    string end_time;    //响应结束时间
    qlibc::QData logicalStripata;   //逻辑灯带数据
    qlibc::QData whiteListData;
    std::thread* updateWhiteListThread;
    std::map<string, stripLight> stripLightContainer;   //灯带管理<deviceid, stripLight>
    std::recursive_mutex Mutex;
    long timeMomentRecord{0};
    static lightManage* Instance;

    lightManage(){
        loadStripLightsContainer();
        //创建线程定时更新白名单
        updateWhiteListThread = new thread([this](){
            qlibc::QData deviceRequest, deviceResponse;
            deviceRequest.setString("service_id", "whiteListRequest");
            deviceRequest.setValue("request", Json::nullValue);
            while(true){
                if(httpUtil::sitePostRequest("127.0.0.1", 9006, deviceRequest, deviceResponse)){
                    std::lock_guard<std::recursive_mutex> lg(Mutex);
                    this->whiteListData = deviceResponse.getData("response");
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::seconds(10));
        });
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

    //物理灯带配置
    void configPhysicalStrip(const StripParamType& sp);

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

    //获取时间精确到微秒
    long getNowTime();
};

#endif