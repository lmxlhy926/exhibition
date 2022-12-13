//
// Created by 78472 on 2022/7/20.
//

#ifndef EXHIBITION_SNADDRESSMAP_H
#define EXHIBITION_SNADDRESSMAP_H

#include <map>
#include <mutex>
#include "qlibc/QData.h"
using namespace std;

/**
 * 功能：
 *      1. 为sn自动分配地址
 *      2. 删除sn对应的地址条目
 *      3. sn--->address
 *      4. address--->sn
 */
class SnAddressMap {
private:
    map<string, Json::Value> snAddrMap;
    std::recursive_mutex rMutex_;

    SnAddressMap(){
        loadCache2Map();
    }

    static SnAddressMap* instance;

public:
    static SnAddressMap* getInstance(){
        if(instance == nullptr){
            instance = new SnAddressMap();
        }
        return instance;
    }

    //获取分配的单播地址
    qlibc::QData getNodeAssignAddr(string deviceSn);

    //删除对应的条目并更新存储文件
    void deleteDeviceSn(string& deviceSn);

    //获取设备地址列表
    qlibc::QData getAddrList();

    //deviceSn--->unicastAddress
    string deviceSn2Address(string deviceSn);

    //unicastAddress--->deviceSn
    string address2DeviceSn(string address);

private:
    //加载存储的<sn-address>数据到snAddMap中
    void loadCache2Map();

    //将map转换为json格式并存储到文件
    void map2JsonDataAndSave2File();

    //数值转换为节点地址
    string intAddr2FullAddr(unsigned int i);

    //插入条目，如果之前有此deviceSn则先删除后插入
    void insert(string& deviceSn, unsigned int intAddr);

    //找到最小可用数值，将可用数值转换为地址
    string getAddress(string& deviceSn);
};

#endif //EXHIBITION_SNADDRESSMAP_H
