//
// Created by 78472 on 2022/6/2.
//
#include "common.h"
#include "common/httplib.h"

qlibc::QData DownCommandData::getContorlData(qlibc::QData &deviceList) {
   ssize_t num = deviceList.size();
   for(Json::ArrayIndex i = 0; i < num; ++i){
       qlibc::QData item = deviceList.getArrayElement(i);
       if(match(item)){
           qlibc::QData controlData;
           controlData.setString("device_id", item.getString("device_id"));
           controlData.setString("sourceSite", item.getString("sourceSite"));
           controlData.putData("command_list", item.getData("command_list"));
           return controlData;
       }
   }
}

bool DownCommandData::match(qlibc::QData &item) {
    return true;
}



