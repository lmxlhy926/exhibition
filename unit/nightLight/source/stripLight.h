
#ifndef NIGHTLIGHT_STRIPLIGHT_H
#define NIGHTLIGHT_STRIPLIGHT_H

#include <string>
#include "qlibc/QData.h"
#include <map>
using namespace std;

//坐标点位
struct CoordPointType{
    double x;   //x坐标
    double y;   //y坐标
};

//灯带灯光控制参数
struct NightLightCtlParamType{
    uint night2Light_brightness;         //亮度
    uint night2Light_color_temperature;  //色温
    uint night2Light_swithTime;          //渐亮时间
    uint light2Night_brightness;         //亮度
    uint light2Night_color_temperature;  //色温
    uint light2Night_swithTime;          //渐亮时间
};

//逻辑灯带参数
struct LogicalStripType{
    bool enable;                            //此段是否开启跟随功能
    string roomNo;                          //灯带房间号
    string logicalName;                     //逻辑灯带名称
    CoordPointType start;                   //灯带起始位置
    CoordPointType end;                     //灯带结束位置
    uint startChipNum;                      //起始控制编号
    uint endChipNum;                        //终止控制编号
    uint singleCtlChipNum;                  //受控芯片数量
    double matchDistance;                   //匹配距离
    double offsetDistance;                  //提前偏移量 (cm)
    NightLightCtlParamType lightParam;      //亮度色温控制参数
};

enum class FunctionCode{
    commonLight,    //普通灯带
    nightLight      //夜灯
};

//实体灯带参数
struct StripParamType{
    string deviceId;            //灯带唯一序列号
    uint totalLength;           //灯带总长度     （cm）
    uint singleChipLength;      //单芯片受控长度  (cm)
    uint totalChips;            //灯控芯片数量
    FunctionCode funCode;       //功能码
};


/**
 * 参数说明：
 *      * 一个灯带拥有一个唯一的序列号
 *      * 一个灯带拥有一套唯一参数：总长度、单芯片长度、芯片数量
 *      * 一个灯带可以设为普通灯带或者夜灯模式。普通灯带模式时，控制整条灯带。
 * 
 *      * 灯带可以划分为多个逻辑段，按段开启/关闭跟随功能。 
 *              一条灯带可以属于多个区域
 *              一条灯带在一个区域可以分为多截。
 *              普通灯带模式时，如果开启按段跟随。只要按段跟随的段对应的控制范围内有人，则灯带亮。所有段都没人，则灯带暗。
*/

using RadarPointsType = std::map<string, std::map<string, CoordPointType>>;  //<房间号, <targetNo, coordPoint>>

/**
 * 逻辑灯带类：包含实体灯带属性、逻辑段灯带属性。
*/
class stripLight{
private:
    StripParamType physicalStrip;   //所属的实体灯带
    std::multimap<string, LogicalStripType> logicalStripMap;    //<房间编号，逻辑灯带段属性>
public:
    stripLight(Json::Value nightStripProperty){
        init(nightStripProperty);
    }

    //增加新的逻辑段
    void addNewLogicStrip(Json::Value nightStripProperty);

    //计算点位是否落在灯带范围内，对灯带相应的段进行控制
    void handleRadarPoints(const RadarPointsType& allRoomAreaPoints);

private:
    //初始灯带参数
    void init(Json::Value nightStripProperty);

    //保存逻辑灯带
    void storeLogicStrip();

    //获取交叉点
    CoordPointType getCrossPoint(CoordPointType start, CoordPointType end, CoordPointType point);

    //比较点位和交叉点，判断是否在灯带控制范围内
    bool isInValidRange(CoordPointType point, CoordPointType crossPoint, double matchDistance);
    
    //获取芯片控制编号
    uint getCtrlChipIndex(CoordPointType start, CoordPointType crossPoint, double offsetDistance,
                          uint singleChipLength, uint startChipNum, uint endChipNum);

    //匹配点位和逻辑灯带
    void matchControl(std::map<string, CoordPointType> roomAreaPoints, LogicalStripType logicalStrip);
};


#endif

