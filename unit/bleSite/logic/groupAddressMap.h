//
// Created by 78472 on 2022/8/29.
//

#ifndef EXHIBITION_GROUPADDRESSMAP_H
#define EXHIBITION_GROUPADDRESSMAP_H

#include <map>
#include <mutex>
#include "qlibc/QData.h"
using namespace std;

class groupAddressMap {
private:
    map<string, Json::Value> groupAddrMap;
    std::recursive_mutex rMutex_;

    groupAddressMap(){
        loadCache2Map();
    }

    static groupAddressMap* instance;

public:
    static groupAddressMap* getInstance(){
        if(instance == nullptr){
            instance = new groupAddressMap();
        }
        return instance;
    }

    //分配组地址
    string assignGroupAddr(string groupName);

    //删除对应的条目并更新存储文件
    void deleteGroupItem(string& groupName);

    //获取组列表
    qlibc::QData getGroupList();

    //groupName--->groupAddress
    string groupName2Address(string groupName);

    //groupAddress--->groupName
    string GroupAddr2GroupName(string groupAddr);

private:
    //加载数据
    void loadCache2Map();

    //将map转换为json格式并存储到文件
    void map2JsonDataAndSave2File();

    //数值转换为组地址
    string intAddr2FullAddr(unsigned int i);

    //插入条目，如果之前有此条目则先删除后插入
    void insert(string& groupName, unsigned int intAddr);

    //找到最小可用数值，将可用数值转换组为地址
    string getAddress(string& deviceSn);
};

#endif //EXHIBITION_GROUPADDRESSMAP_H
