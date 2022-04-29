//
// Created by 78472 on 2022/4/29.
//

#include "QData.h"
#include <sstream>
#include <fstream>

QData::QData() {
    _value.reset(new Json::Value(Json::nullValue));
}

QData::QData(const string &source) {
    _value.reset(new Json::Value(Json::nullValue));
    parseJson(source, *_value);
}

QData::QData(const Json::Value &val) {
    _value.reset(new Json::Value(val));
}

QData::QData(const QData& data) {
    _value.reset(new Json::Value(*data.asValue()));
}


bool QData::contains(const string &src, const string &dest) {
    if(src.find(dest) != std::string::npos) return true;
    return false;
}

bool QData::parseJson(const string &srcStr, Json::Value &destValue) {
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

Json::Value QData::parseJson(const string &srcStr) {
    Json::Value destValue(Json::nullValue);
    parseJson(srcStr, destValue);
    return destValue;
}

bool QData::valueToJsonString(const Json::Value &obj, string &ret) {
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

bool QData::parseFromFile(const string &fileNamePath, Json::Value &value) {
    ifstream infile(fileNamePath,std::ios::in);
    Json::CharReaderBuilder b;
    JSONCPP_STRING errs;
    if (!parseFromStream(b, infile, &value, &errs)) {
        value = Json::nullValue;
        return false;
    }
    return true;
}

Json::Value QData::parseFromFile(const string &filePathName) {
    Json::Value value;
    parseFromFile(filePathName, value);
    return value;
}

bool QData::writeToFile(const string &filePathName, const Json::Value &value) {
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

std::shared_ptr<Json::Value> QData::asValue() const{
    return _value;
}

Json::ArrayIndex QData::size() const {
   return _value->size();
}

Json::ValueType QData::type() const {
    return _value->type();
}

bool QData::empty() const {
    return _value->empty();
}

void QData::clear() {
    if(_value->isNull() || _value->isObject() || _value->isArray()){
        _value->clear();
    }
}

void QData::removeMember(const string &key) {
    if(_value->isNull() || _value->isObject()){
        _value->removeMember(key.c_str());
    }
}

Json::Value::Members QData::getMemberNames() {
    if(_value->isNull() || _value->isObject()){
        return _value->getMemberNames();
    }
    return Json::Value::Members();
}

void QData::toJsonString(string &str, bool expand) {
    if(expand)
        str = _value->toStyledString();
    else
        valueToJsonString(*_value, str);
}

std::string QData::toJsonString(bool expand) {
    string str;
    toJsonString(str, expand);
    return str;
}

void QData::loadFromFile(const string &filePathName) {
   _value.reset(new Json::Value(parseFromFile(filePathName)));
}

void QData::saveToFile(const string &filePathName) {
    writeToFile(filePathName, *_value);
}

bool QData::getBool(const string &key, bool defValue) const {
    return false;
}




