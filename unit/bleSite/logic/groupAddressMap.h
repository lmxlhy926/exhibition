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

    //创建分组
    bool createGroup(string groupName);

    //删除分组
    void deleGroup(string& groupId);

    //分组重命名
    bool reNameGroup(string& groupId, string& groupName);

    //设备加入分组
    void addDevice2Group(string& groupId, string& deviceSn);

    //设备从分组剔除
    void removeDeviceFromGroup(string& groupId, string& deviceSn);

    //将设备从所有组中移除
    void removeDeviceFromAnyGroup(string& deviceSn);

    //返回分组列表
    qlibc::QData getGroupList();

    //分组是否存在
    bool isGroupExist(string& groupId);

    //groupName--->groupAddressID
    string groupName2GroupAddressId(string groupName);

    //groupAddress--->groupName
    string groupAddressId2GroupName(string groupAddrId);

private:
    //加载数据
    void loadCache2Map();

    //将map转换为json格式并存储到文件
    void map2JsonDataAndSave2File();

    //数值转换为组地址
    string intAddr2FullAddr(unsigned int i);

    //插入条目
    void insert(string& groupName, unsigned int intAddr);
};

#endif //EXHIBITION_GROUPADDRESSMAP_H
