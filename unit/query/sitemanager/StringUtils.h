//
// Created by WJG on 2022-4-25.
//

#ifndef SITEMANAGERSERVICE_STRINGUTILS_H
#define SITEMANAGERSERVICE_STRINGUTILS_H

#include "qlibc/jsoncpp/json.h"

#define MAX_STR_BUFF_SIZE 2048

typedef std::vector<std::string> str_vector;

static bool export_json_err_ = true;
class StringUtils{

public:
    static void formatString(std::string &str, const char *fmt, ...);
    static bool isValidJsonString(const std::string &srcstr);
    static Json::Value parseJson(const std::string &srcstr);
    static Json::Value parseFromFile(const std::string &fn);
    static std::string valueToJsonString(const Json::Value &obj);
    static void writeToFile(const std::string &filename, const Json::Value &value);
    static std::string removeChar(const std::string &src, const unsigned char c);
    static void splitString(const std::string& s, const std::string& c,str_vector &v);
    static void replace(std::string &str, const std::string &key, const std::string &val);
};

#endif //SITEMANAGERSERVICE_STRINGUTILS_H
