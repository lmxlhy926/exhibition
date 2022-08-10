//
// Created by WJG on 2022-4-25.
//

#include <cstring>
#include <cstdarg>
#include <memory>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <functional>
#include <regex>
#include <fstream>

#include "StringUtils.h"

bool StringUtils::isValidJsonString(const std::string &srcstr){
    int i = 0;
    int len = srcstr.size();
    while( (isspace(srcstr[i])) && (i<len) )
        i++;
    return (srcstr[i]=='{') || (srcstr[i]=='[');
}

Json::Value StringUtils::parseJson(const std::string &srcstr) {
    Json::Value dstVal;
    Json::CharReaderBuilder b;
    Json::CharReader* reader(b.newCharReader());
    const char* str = srcstr.c_str();
    JSONCPP_STRING errs;
    bool ok = reader->parse(str, str + srcstr.size(), &dstVal, &errs);
    if (!ok){
        if(export_json_err_)
            std::cout << "!!!ERROR when parsing as bellows::\n"<< srcstr << std::endl << errs ;
        dstVal = Json::nullValue;
    }
    delete reader;
    return dstVal;
}

Json::Value StringUtils::parseFromFile(const std::string &fn) {
    Json::Value dstVal;
    std::ifstream infile(fn, std::ios::in);
    Json::CharReaderBuilder b;
    JSONCPP_STRING errs;
    if (!parseFromStream(b, infile, &dstVal, &errs)) {
        if(export_json_err_)
            std::cout << "!!!ERROR when parsing as bellows::\n" << fn << std::endl << errs;
        dstVal = Json::nullValue;
    }
    infile.close();
    return dstVal;
}

std::string StringUtils::valueToJsonString(const Json::Value& obj){
    std::string ret;

    if(obj.isNull())
        ret = "";
    switch(obj.type())
    {
        case Json::intValue:
            formatString(ret, "%d", obj.asInt64());
            break;
        case Json::uintValue:
            formatString(ret, "%u", obj.asUInt64());
            break;
        case Json::realValue:
            formatString(ret, "%u", obj.asDouble());
            break;
        case Json::stringValue:
            ret = obj.asString();
            break;
        case Json::booleanValue:
            ret = obj.asBool()?"true":"false";
            break;
        case Json::arrayValue:
        case  Json::objectValue:
            {
                std::ostringstream os;
                Json::StreamWriterBuilder builder;
                builder.settings_["indentation"] = "";
                std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
                writer->write(obj, &os);
                ret = os.str();
            }
            break;
        case Json::nullValue:
        default:
            ret = "";
            break;
    }

    return ret;
}

void StringUtils::formatString(std::string &str, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char buff[MAX_STR_BUFF_SIZE];
    vsnprintf(buff, MAX_STR_BUFF_SIZE , fmt, ap);
    va_end(ap);
    str = buff;
}

void StringUtils::writeToFile(const std::string &filename, const Json::Value& value) {
    std::string str;
    str = valueToJsonString(value);
    std::ofstream outfile(filename, std::ios::out);
    if(outfile.is_open()) {
        outfile << str << std::endl;
        outfile.close();
    }
}

std::string StringUtils::removeChar(const std::string& s, const unsigned char c){
    if(s.empty())
        return "";

    int count = static_cast<int>(s.length());
    const char *cc = s.c_str();
    char *result = (char*)malloc(count+1);
    memset(result, 0x00, count+1);
    int j = 0;
    for(auto i=0;i<count;i++){
        if(cc[i] == c)
            continue;

        result[j] = cc[i];
        j++;
    }

    std::string rs(result);
    free(result);
    return rs;
}

void StringUtils::splitString(const std::string& s, const std::string& c, str_vector &v)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }

    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

void StringUtils::replace(std::string &str, const std::string &key, const std::string &val){
    if(str.empty() || key.empty())
        return;

    int place = 0;
    do
    {
        place = (int)str.find(key,place);
        if (place == -1)
            break;

        str.replace(place, key.size(), val);
        place++;
    }while(place!=-1);
}
