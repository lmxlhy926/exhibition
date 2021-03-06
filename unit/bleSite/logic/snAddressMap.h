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
 *      1. 为sn自动分配地址，保证地址唯一对应，自动增加条目
 *      2. 返回分配地址字符串
 *      3. 返回绑定的设备列表
 *      4. 删除设备条目
 */
class SnAddressMap {
private:
    map<string, int> snAddrMap;
    std::recursive_mutex mutex_;
public:
    SnAddressMap(){
        loadCache2Map();
    }

    //获取节点分配地址字符串并更新存储文件
    qlibc::QData getNodeAssignAddr(string deviceSn);

    //删除对应的条目并更新存储文件
    void deleteDeviceSn(string& deviceSn);

    //获取设备列表
    qlibc::QData getDeviceList();

private:
    //加载存储的<sn-address>数据到snAddMap中
    void loadCache2Map();

    //插入条目，如果之前有此deviceSn则先删除后插入
    void insert(string& deviceSn, unsigned int intAddr);

    string intAddr2FullAddr(unsigned int i);

    //将map转换为json格式并存储到文件
    void map2JsonDataAndSave2File();

    //返回最小的可用数值
    string getAddress(string& deviceSn);
};

#endif //EXHIBITION_SNADDRESSMAP_H
