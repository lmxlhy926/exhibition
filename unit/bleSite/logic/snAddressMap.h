//
// Created by 78472 on 2022/7/20.
//

#ifndef EXHIBITION_SNADDRESSMAP_H
#define EXHIBITION_SNADDRESSMAP_H

#include <map>
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
public:
    SnAddressMap(){
        init();
    }

    //获取节点分配地址字符串
    qlibc::QData getNodeAssignAddr(string deviceSn);

private:
    //加载存储的<sn-address>数据到snAddMap中
    void init();

    void save2File();

    //获取节点分配地址字符串
    unsigned int getAddrInt(string& deviceSn);
};


#endif //EXHIBITION_SNADDRESSMAP_H
