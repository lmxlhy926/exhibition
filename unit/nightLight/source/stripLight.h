
#ifndef NIGHTLIGHT_STRIPLIGHT_H
#define NIGHTLIGHT_STRIPLIGHT_H

#include <string>
#include <map>
#include <mutex>
#include "qlibc/QData.h"
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
    string logicalStripName;                    //逻辑灯带名称
    string roomNo;                              //灯带房间号
    CoordPointType start;                       //灯带起始位置
    CoordPointType end;                         //灯带结束位置
    uint startChipNum;                          //起始控制编号
    uint endChipNum;                            //终止控制编号
};

enum class FunctionCode{
    commonLight,    //普通灯带
    nightLight      //夜灯
};

//实体灯带参数
struct StripParamType{
    string device_id;                        //灯带唯一序列号
    uint strip_length;                       //灯带总长度     （cm）
    uint led_spacing;                        //单芯片受控长度  (cm)
    uint totalChips;                         //灯控芯片数量
    double lighting_range;                   //亮灯长度
    uint singleCtlChipNum;                   //受控芯片数量
    double sensing_distance;                 //感应距离
    double focus_offset;                     //提前偏移量 (cm)
    NightLightCtlParamType lightParam;       //亮度色温控制参数
    FunctionCode funCode;                    //功能码
    bool valid{false};                       //有效数据？
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

using RadarPointsType = std::map<string, std::vector<CoordPointType>>;  //<房间号, coordPoint>

/**
 * 逻辑灯带类：包含实体灯带属性、逻辑段灯带属性。
*/
class stripLight{
private:
    StripParamType physicalStrip;   //所属的实体灯带
    std::recursive_mutex Mutex;
    std::map<string, std::vector<LogicalStripType>> logicalStripMap;    //<执行对象名称，逻辑灯带段属性>
    std::vector<uint> chipOpendIndex;   //打开的芯片编号
public:
    //单参构造函数
    stripLight(StripParamType ps){
        physicalStrip = ps;
    }

    //拷贝构造函数
    stripLight(const stripLight& sl){
        this->physicalStrip = sl.physicalStrip;
        this->logicalStripMap = sl.logicalStripMap;
        this->chipOpendIndex = sl.chipOpendIndex;
    }

    //增加新的逻辑段或替换之前存在的逻辑段
    bool addExecuteObj(string const& objName, std::vector<LogicalStripType> const& logicalStripVec);

    //删除逻辑段
    bool delExecuteObj(string const& objName);

    //获取逻辑灯带列表
    Json::Value getLogicalStripList();

    //计算点位是否落在灯带范围内，对灯带相应的段进行控制
    void handleRadarPoints(const RadarPointsType& allPoints);

private:
    //获取交叉点坐标
    CoordPointType getCrossPoint(LogicalStripType const& logicalStrip, CoordPointType const& point);

    /**
     * 如果是受控点，则返回灯带控制编号
     * 如果不是受控点，返回-1
    */
    int getCtrlChipIndex(LogicalStripType const& logicalStrip, CoordPointType const& point);

    //依据编号控制灯带
    void controlStrip(std::vector<uint> index2Open, std::vector<uint> index2Close);

    //设置本次需要点亮的芯片标号
    void setChipIndexs2Open(LogicalStripType const& logicalStrip, std::vector<CoordPointType> const& points, std::vector<uint>& chipIndex2Open);

    //点位比较
    bool pointsEqual(CoordPointType first, CoordPointType second);

    //打印点位
    void printPoint(const string& msg, const CoordPointType& point);

    //灯带逻辑属性的Json格式
    Json::Value LogicalStripType2Value(LogicalStripType const& logicalStrip);

    //物理灯带属性信息
    Json::Value physicalStripType2Value(StripParamType const& physicalStrip);
};


#endif

