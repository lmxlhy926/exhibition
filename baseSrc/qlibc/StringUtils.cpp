//
// Created by 78472 on 2022/4/28.
//

#include "StringUtils.h"
#include <memory>
#include <sstream>
#include <fstream>

bool StringUtils::contains(const std::string& src, const std::string& dest) {
    if(src.find(dest) != std::string::npos) return true;
    return false;
}

bool StringUtils::parseJson(const string &srcStr, Json::Value &destValue) {
    Json::CharReaderBuilder b;
    std::unique_ptr<Json::CharReader> reader(b.newCharReader());
    const char* str = srcStr.c_str();
    JSONCPP_STRING errs;
    bool ok = reader->parse(str, str + srcStr.size(), &destValue, &errs);
    if (!ok){
        destValue = Json::nullValue;
        return false;
    }
    return true;
}

Json::Value StringUtils::parseJson(const string &srcStr) {
    Json::Value destValue(Json::nullValue);
    parseJson(srcStr, destValue);
    return destValue;
}

bool StringUtils::valueToJsonString(const Json::Value &obj, string &ret) {
    std::ostringstream os;
    Json::StreamWriterBuilder builder;
    builder.settings_["indentation"] = "";
    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    writer->write(obj, &os);
    if(!os.good()){
        ret = "";
        return false;
    }
    ret = os.str();
    return true;
}

bool StringUtils::parseFromFile(const std::string& fileNamePath, Json::Value &value) {
    ifstream infile(fileNamePath,std::ios::in);
    Json::CharReaderBuilder b;
    JSONCPP_STRING errs;
    if (!parseFromStream(b, infile, &value, &errs)) {
        value = Json::nullValue;
        return false;
    }
    return true;
}

Json::Value StringUtils::parseFromFile(const std::string& filePathName) {
    Json::Value value;
    parseFromFile(filePathName, value);
    return value;
}

bool StringUtils::writeToFile(const std::string& filePathName, const Json::Value &value) {
    string content;
    if(valueToJsonString(value, content)){
        std::ofstream out(filePathName, std::ios::out);
        if(out.is_open()){
            out << content << std::endl;
            return true;
        }
    }
    return false;
}




