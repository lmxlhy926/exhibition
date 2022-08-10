//
// Created by WJG on 2022-4-25.
//

#ifndef SITEMANAGERSERVICE_COMM_DEFINE_H
#define SITEMANAGERSERVICE_COMM_DEFINE_H

#define SPACE_CHAR 32

#define MY_SITE_ID_NAME "site_query_service"

#define SITE_REGISTER_SERVICE_ID "site_register"
#define SITE_QUERY_SERVICE_ID "site_query"
#define SITE_PING_SERVICE_ID "site_ping"

#define SITE_ONOFFLINE_MESSAGE_ID "site_onoffline"

#define CMD_PARAM "param"
#define SITE_IP "ip"
#define SITE_PORT "port"
#define SITE_ID "site_id"
#define SITE_LIST "site_list"
#define SITE_STATUS "site_status"

#define SERVICE_ID "service_id"
#define MESSAGE_ID "message_id"
#define CONTENT_KEY "content"
#define REQUEST_KEY "request"
#define RESPONSE_KEY "response"

#define ON_LINE "online"
#define OFF_LINE "offline"

#define PAYLOAD "payload"
#define RESP_CODE "code"
#define MESSAGE "error"
#define MSG_SUCCESS "ok"
#define MSG_FAIL "fail"
#define RESPON_FAIL_MSG "Request return failed,please check url,token and param."
#define CODE_SUCCESS 0
#define CODE_FAIL 201
#define CODE_PARAM_ERR 202
#define CODE_SITE_NOTFOUND 203
#define CODE_UNSUPPORT 204

#endif //SITEMANAGERSERVICE_COMM_DEFINE_H
