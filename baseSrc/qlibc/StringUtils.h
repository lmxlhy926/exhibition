//
// Created by 78472 on 2022/4/28.
//

#ifndef EXHIBITION_STRINGUTILS_H
#define EXHIBITION_STRINGUTILS_H

#include <string>
#include <json/json.h>

using namespace std;

class StringUtils {
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


#endif //EXHIBITION_STRINGUTILS_H
