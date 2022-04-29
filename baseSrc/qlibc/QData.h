//
// Created by 78472 on 2022/4/29.
//

#ifndef EXHIBITION_QLIBC_H
#define EXHIBITION_QLIBC_H

#include <string>
#include <memory>
#include <mutex>
#include "json/json.h"

using namespace std;

class QData {
private:
    std::shared_ptr<Json::Value> _value;
    std::recursive_mutex _mutex;
public:
    QData();
    explicit QData(const std::string& source);
    explicit QData(const Json::Value& val);
    QData(const QData& data);

public:
    std::shared_ptr<Json::Value> asValue() const;
    Json::ArrayIndex size() const;
    Json::ValueType type() const;
    bool empty() const;
    void clear();
    void removeMember(const std::string& key);
    Json::Value::Members getMemberNames();

    void toJsonString(std::string& str, bool expand = false);
    std::string toJsonString(bool expand = false);
    void loadFromFile(const std::string& filePathName);
    void saveToFile(const std::string& filePathName);

    bool getBool(const std::string& key, bool defValue) const;
    bool getBool(const std::string& key) const;
    QData& setBool(const std::string& key, bool value);

    int getInt(const std::string& key, int defValue) const;
    int getInt(const std::string& key) const;
    QData& setInt(const std::string& key, int val);

    std::string getString(const std::string& key, const std::string& defValue) const;
    std::string getString(const std::string& key) const;
    QData& setString(const std::string& key, const std::string& value);

    bool getData(const std::string& key, QData& data);
    QData getData(const std::string& key) const;
    bool putData(const std::string& key, const QData& data);

    bool getValue(const std::string& key, Json::Value& value) const;
    Json::Value getValue(const std::string& key) const;
    void setValue(const std::string& key, const Json::Value& value);

public:
    /**
    * 判断源字符串中是否含有目标字符串
    * 成功返回true; 失败返回false;
    */
    static bool contains(const std::string& src, const std::string& dest);

    /**
     * 将字符串转换为Json::Value对象
     * 成功返回true; 失败返回false,且destValue为Json::Value(Json::nullValue)
     */
    static bool parseJson(const std::string& srcStr, Json::Value& destValue);

    /**
     * 将字符串转换为Json::Value对象,
     * 失败返回Json::Value(Json::nullValue)
     */
    static Json::Value parseJson(const std::string& srcStr);

    /**
     * 将Json::Value对象转换为字符串，失败则ret为空。
     * 成功返回true; 失败返回false,且ret为空;
     */
    static bool valueToJsonString(const Json::Value& obj, std::string& ret);

    /**
     * 将从文件读取的内容转换为Json::Value对象
     * 成功返回true; 失败返回fasle,且value为Json::Value(Json::nullValue)
     */
    static bool parseFromFile(const std::string& fileNamePath, Json::Value& value);

    /**
     * 将从文件读取的内容转换为Json::Value对象
     * 失败返回Json::Value(Json::nullValue)
     */
    static Json::Value parseFromFile(const std::string& filePathName);

    /**
     * 将value对象转换为字符串后写入到文件中（truncate方式）
     * 成功返回true;失败返回false;
     */
    static bool writeToFile(const std::string& filePathName, const Json::Value& value);

};


#endif //EXHIBITION_QLIBC_H
