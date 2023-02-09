//
// Created by 78472 on 2022/6/2.
//

#ifndef EXHIBITION_DOWNCOMMAND_H
#define EXHIBITION_DOWNCOMMAND_H

#include <string>
#include <iostream>
#include "log/Logging.h"
#include "qlibc/QData.h"

using namespace std;

class DownCommandData {
private:
     string type;               //控制对象类型
     string action;             //控制动作
     string code;               //控制设备标识
     string delay;              //延时
     qlibc::QData inParams;     //控制参数

public:
    explicit DownCommandData(qlibc::QData &data) {
        qlibc::QData request = data.getData("request");
        type = request.getString("type");
        action = request.getString("action");
        code = toUpper(hump2Underline(request.getString("code")));
        delay = request.getString("delay");
        inParams = request.getData("inParams");
        LOG_PURPLE << "code: " << code << "---" << "area: " << inParams.getString("area")
                   << "---" << "kind: " << inParams.getString("kind")
                   << "---" << "deviceCode: " << inParams.getString("deviceCode");
    }

    //依据下发命令、设备列表定位设备，返回控制指令
    qlibc::QData getContorlData(qlibc::QData& deviceList);

    //按区域、类型进行匹配，定位设备
    bool fuzzyMatch(qlibc::QData& item);

    //按设备号，或者区域、类型进行匹配
    bool match(qlibc::QData& item);

    //构造控制命令列表
    qlibc::QData buildCommandList(qlibc::QData& data);

    //预定命令列表中是否包含该命令
    bool commandMatch(string& key);

    //驼峰转下划线
    static string hump2Underline(string in);

    //小写转大写
    static string toUpper(string in);
};

#endif //EXHIBITION_DOWNCOMMAND_H
