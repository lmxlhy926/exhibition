//
// Created by 78472 on 2022/5/11.
//

#include "serviceRequestHandler.h"
#include "common/httpUtil.h"
#include "siteService/service_site_manager.h"
using namespace servicesite;


int whiteList_service_get_handler(const Request& request, Response& response){
    std::cout << "===>whiteList_service_request_handler: " << request.body <<  std::endl;

    qlibc::QData whiteListData = whiteListParamUtil::getInstance()->getWhiteList();
    response.set_content(whiteListData.toJsonString(), "text/json");

    return 0;
}


int whiteList_service_put_handler(const Request& request, Response& response){

    qlibc::QData ret;
    qlibc::QData data(request.body);
    if(data.type() == Json::nullValue){
        ret.setInt("code", 201);
        ret.setString("msg", "request format is not correct");
        response.set_content(ret.toJsonString(), "text/json");
        return 0;
    }

    qlibc::QData whiteListData = data.getData("request");
    whiteListParamUtil::getInstance()->saveWhiteListData(whiteListData);

    ret.setInt("code", 200);
    ret.setString("msg", "success");
    response.set_content(ret.toJsonString(), "text/json");

    return 0;
}
