//
// Created by 78472 on 2022/8/29.
//

#ifndef EXHIBITION_GROUPADDRESSMAP_H
#define EXHIBITION_GROUPADDRESSMAP_H

#include <map>
#include <mutex>
#include "qlibc/QData.h"
using namespace std;

class GroupAddressMap {
private:
    map<string, Json::Value> groupAddrMap;
    std::recursive_mutex rMutex_;

    GroupAddressMap(){
        loadCache2Map();
    }

    static GroupAddressMap* instance;

public:
    static GroupAddressMap* getInstance(){
        if(instance == nullptr){
            instance = new GroupAddressMap();
        }
        return instance;
    }

    //获取组地址
    string getGroupAddr(string groupName);

    //设备加入分组
    void addDevice2Group(string& groupName, string& deviceSn);

    //删除对应的条目并更新存储文件
    void deleteGroupItem(string& groupName);

    //获取组列表
    qlibc::QData getGroupList();

    //groupName--->groupAddress
    string groupName2Address(string groupName);

    //groupAddress--->groupName
    string groupAddr2GroupName(string groupAddr);

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
    string getAddress(string& groupName);
};

#endif //EXHIBITION_GROUPADDRESSMAP_H
