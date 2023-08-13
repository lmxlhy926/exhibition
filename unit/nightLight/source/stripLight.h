
#ifndef NIGHTLIGHT_STRIPLIGHT_H
#define NIGHTLIGHT_STRIPLIGHT_H

#include <string>
#include "qlibc/QData.h"
#include <map>
using namespace std;

struct CoordPointType{
    double x;   //x坐标
    double y;   //y坐标
};

struct LightParamType{
    uint night2Light_brightness;         //亮度
    uint night2Light_color_temperature;  //色温
    uint night2Light_swithTime;         //渐亮时间
    uint light2Night_switchTime;        //渐暗时间
};

struct StripPositionType{
    bool enable;                    //此段是否开启跟随功能
    uint offset;                    //控制距离偏移量
    CoordPointType start;           //灯带起始位置
    CoordPointType end;             //等待结束位置
    LightParamType lightParam;      //亮度色温控制参数
};

struct StripParamType{
    uint totalLength;      //灯带总长度   （cm）
    uint uintCtrlLength;   //单位受控长度  (cm)
    uint ctrlLength;       //单次受控长度 （cm）
    uint ctrlDistance;     //受控距离范围 （cm）
};

struct ChipParamType{
    uint totalChips;      //灯控芯片数量
    uint chipCtrlNum;     //单次芯片控制数量
};

enum class FunctionCode{
    commonLight,    //普通灯带
    nightLight      //夜灯
};

/**
 * 参数说明：
 *      * 一个灯带拥有一个唯一的序列号
 *      * 一个灯带拥有一套参数；依据设置的灯带参数，计算灯控芯片总数量和每次控制的灯控芯片数量
 *      * 一个灯带可以设为普通灯带或者夜灯模式。普通灯带模式时，控制整条灯带。
 * 
 *      * 灯带可以按段开启/关闭跟随功能。 
 *              一条灯带可以属于多个区域
 *              一条灯带在一个区域可以分为多截。
 *              普通灯带模式时，如果开启按段跟随。只要按段跟随的段对应的控制范围内有人，则灯带亮。所有段都没人，则灯带暗。
*/

using PointSequenceType = std::map<string, std::map<string, CoordPointType>>;  //<area, <targetNo, coordPoint>>
class stripLight{
private:
    string deviceId;                //灯带唯一序列号
    StripParamType stripParam;      //灯带参数
    ChipParamType chipParam;        //灯控芯片数量参数  
    FunctionCode funCode;           //功能码
    std::multimap<string, StripPositionType> areaAndPostion;    //<区域位置，灯带坐标>
public:
    stripLight(Json::Value stripDevices){
        init(stripDevices);
    }

private:
    //初始灯带参数
    void init(Json::Value stripDevices);

    //计算点位是否落在灯带范围内，对灯带相应的段进行控制
    void calculateAndContol(PointSequenceType pointSequence);
};


#endif

