//
// Created by 78472 on 2022/5/18.
//

#include "util.h"
#include "paramConfig.h"

bool util::getTvInfo(string& tvMac, string& tvName, string& tvModel){
    qlibc::QData baseInfoData = configParamUtil::getInstance()->getBaseInfo();
    tvMac = baseInfoData.getString("deviceMac");
    tvName = baseInfoData.getString("tv_name");
    tvModel = baseInfoData.getString("tv_model");

    if(tvMac.empty() || tvName.empty() || tvModel.empty()){
        qlibc::QData tvRequest, tvResponse;
        tvRequest.setString("service_id", "get_tv_mac");
        tvRequest.putData("request", qlibc::QData());

        auto ret = httpUtil::sitePostRequest(ADAPTER_IP, ADAPTER_PORT, tvRequest, tvResponse);
        if(ret){
            if(tvResponse.getInt("code") == 0){
                tvMac = tvResponse.getData("response").getString("tv_mac");
                tvName = tvResponse.getData("response").getString("tv_name");
                tvModel = tvResponse.getData("response").getString("tv_model");

                baseInfoData.setString("deviceSn", tvMac);
                baseInfoData.setString("deviceMac", tvMac);
                baseInfoData.setString("tv_name", tvName);
                baseInfoData.setString("tv_model", tvModel);
                configParamUtil::getInstance()->saveBaseInfo(baseInfoData);
                return true;
            }
        }
        return false;
    }
    return true;
}

bool util::getTvInfo(qlibc::QData& tvInfo){
    string tvMac, tvName, tvModel;
    bool ret = getTvInfo(tvMac, tvName, tvModel);
    if(ret){
        qlibc::QData params;
        params.setString("tvMac", tvMac);

        tvInfo.setString("funcName", "deviceDataReport");
        tvInfo.setString("deviceType", "tvMac");
        tvInfo.setString("area", "");
        tvInfo.setString("deviceName", "");
        tvInfo.setString("eventName", "tvMac");
        tvInfo.setValue("params", params.asValue());

        return true;
    }
    return false;
}