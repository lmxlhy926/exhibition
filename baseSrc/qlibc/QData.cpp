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

QData::QData(const char* str, int size){
    _value.reset(new Json::Value(Json::nullValue));
    parseJson(str, size, *_value);
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

bool QData::parseJson(const char* srcStr, int srcSize, Json::Value& destValue){
    Json::CharReaderBuilder b;
    std::unique_ptr<Json::CharReader> reader(b.newCharReader());
    const char* str = srcStr;
    JSONCPP_STRING errs;
    bool ok = reader->parse(str, str + srcSize, &destValue, &errs);
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

QData& QData::setInitData(const QData& data){
    Json::Value v = *data.asValue();
    setInitValue(v);
    return *this;
}

QData& QData::setInitValue(const Json::Value& value){
    *_value = value;
    return *this;
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
    if(!_value->isObject() || key.empty()) return defValue;
    Json::Value v = _value->get(key, Json::Value());
    if(v.isBool())  return v.asBool();
    return defValue;
}

bool QData::getBool(const std::string& key) const{
    return getBool(key, false);
}

QData &QData::setBool(const string &key, bool value) {
   if(!_value->isObject() || key.empty())   return *this;
    (*_value)[key] = value;
    return *this;
}

int QData::getInt(const string &key, int defValue) const {
    if(!_value->isObject() || key.empty())  return defValue;
    Json::Value v = (*_value).get(key, Json::Value());
    if(v.isInt())   return v.asInt();
    return defValue;
}

int QData::getInt(const string &key) const {
    return getInt(key, -1);
}

QData &QData::setInt(const string &key, int val) {
    if((!_value->isNull() && !_value->isObject()) || key.empty())
        return *this;
    (*_value)[key] = val;
    return *this;
}

std::string QData::getString(const string &key, const string &defValue) const {
    if(!_value->isObject() || key.empty())  return defValue;
    Json::Value v = _value->get(key, Json::Value());
    if(v.isString())    return v.asString();
    return defValue;
}

std::string QData::getString(const string &key) const {
   return getString(key, "");
}

QData &QData::setString(const string &key, const string &value) {
    if((!_value->isNull() && !_value->isObject()) || value.empty())
        return *this;
    (*_value)[key] = value;
    return *this;
}

void QData::getData(const string &key, QData &data) const{
    if(!_value->isObject() || key.empty()){
        data.setInitValue(Json::Value());
        return;
    }
    Json::Value v = _value->get(key, Json::Value());
    data.setInitValue(v);
}

QData QData::getData(const string &key) const {
    QData data;
    getData(key, data);
    return data;
}

void QData::putData(const string &key, const QData &data) {
    if((!_value->isNull() && !_value->isObject()) || key.empty())
        return;
    (*_value)[key] = *data.asValue();
}

void QData::getValue(const string &key, Json::Value &value) const {
    if(!_value->isObject() || key.empty()){
        value = Json::Value();
        return;
    }
    value = _value->get(key, Json::Value());
}

Json::Value QData::getValue(const string &key) const {
    Json::Value v;
    getValue(key, v);
    return v;
}

void QData::setValue(const string &key, const Json::Value &value) {
    if(!_value->isObject() || key.empty())  return;
    (*_value)[key] = value;
}


