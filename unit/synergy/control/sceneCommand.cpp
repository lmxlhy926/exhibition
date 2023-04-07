//
// Created by 78472 on 2022/10/9.
//

#include "sceneCommand.h"
#include "common/httpUtil.h"
#include "log/Logging.h"
#include "../param.h"

bool SceneCommand::sendCmd(qlibc::QData& siteResponse){
    qlibc::QData req, siteRequest;
    if (action == "constantBrightnessModeStart") {  //开启恒定照度
        req.setBool("active_switch", true);
        req.setInt("target_illumination", inParams.getInt("brightness"));
        req.setInt("target_temperature", inParams.getInt("colourTemperature"));
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "constant_illuminance");
        siteRequest.putData("request", req);

    } else if(action == "constantBrightnessModeStop"){  //关闭恒定照度
        req.setBool("active_switch", false);
        req.setInt("target_illumination", 0);
        req.setInt("target_temperature", 0);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "constant_illuminance");
        siteRequest.putData("request", req);

    }else if (action == "constantBrightnessModeFlag") {    //设置恒定照度
        string flag = inParams.getString("flag");
        bool active_switch;
        if (flag == "0") {
            active_switch = true;
        } else {
            active_switch = false;
        }

        req.setBool("active_switch", active_switch);
        req.setInt("target_illumination", 0);
        req.setInt("target_temperature", 0);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));

        siteRequest.setString("service_id", "constant_illuminance");
        siteRequest.putData("request", req);

    }else if(action == "comfortableDinnerModeStart"){   //温馨就餐开启
        req.setBool("active_switch", true);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "dining");
        siteRequest.putData("request", req);

    }else if(action == "comfortableDinnerModeStop"){    //温馨就餐关闭
        req.setBool("active_switch", false);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "dining");
        siteRequest.putData("request", req);

    }else if(action == "readModeStart"){    //阅读模式开启
        req.setBool("active_switch", true);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "reading");
        siteRequest.putData("request", req);

    }else if(action == "readModeStop"){     //阅读模式关闭
        req.setBool("active_switch", false);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "reading");
        siteRequest.putData("request", req);

    }else if(action == "cookModeStart"){    //烹饪模式开启
        req.setBool("active_switch", true);
        req.setInt("target_temperature", inParams.getInt("colourTemperature"));
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "cooking");
        siteRequest.putData("request", req);

    }else if(action == "cookModeStop"){     //烹饪模式关闭
        req.setBool("active_switch", false);
        req.setInt("target_temperature", inParams.getInt("colourTemperature"));
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "cooking");
        siteRequest.putData("request", req);

    }else if(action == "enterHouseholdModeStart"){  //回家模式客厅有人
        req.setBool("active_switch", true);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "enter_home");
        siteRequest.putData("request", req);

    }else if(action == "enterHouseholdNoPersonModeStart"){  //回家模式无人
        req.setBool("active_switch", false);
        req.setInt("areaCode", atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "enter_home");
        siteRequest.putData("request", req);

    }else if(action == "viewingSceneStart"){  //开启观影场景
        req.setBool("active_switch", true);
        req.setInt("areaCode",atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "movieTime");
        siteRequest.putData("request", req);

    }else if(action == "viewingSceneStop"){
        req.setBool("active_switch", false);
        req.setInt("areaCode",atoi(inParams.getString("area").c_str()));
        siteRequest.setString("service_id", "movieTime");
        siteRequest.putData("request", req);

    }else {
        LOG_RED << action << " is not supported.....";
        return false;
    }

    LOG_GREEN << "sceneCmd: " << siteRequest.toJsonString();
    return httpUtil::sitePostRequest("127.0.0.1", 9011, siteRequest, siteResponse);
}
