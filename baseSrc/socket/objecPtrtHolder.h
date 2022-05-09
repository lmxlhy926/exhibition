//
// Created by 78472 on 2022/5/8.
//

#ifndef EXHIBITION_OBJECPTRTHOLDER_H
#define EXHIBITION_OBJECPTRTHOLDER_H

#include <map>
#include <mutex>
#include <functional>
#include <string>

using namespace std;

/*
 * 管理指针指向的对象，从无序容器中移除记录时，释放内存资源
 */
template<typename T>
class objectPtrHolder {
private:
    std::map<const std::string, T *>  objectPtrMap;
    std::recursive_mutex mutex_;

public:
     objectPtrHolder()= default;

    //析构时释放所有被管理的对象的资源
    ~objectPtrHolder();

    //添加要管理的对象的指针
    void appendNew(const string& key, T* objPtr);

    /**
     * 返回key标识的对象的指针
     * @param key
     * @return 不存在则返回nullptr
     */
    T* findObject(const string& key);

    //是否存在key标识的对象
    bool existObject(const string& key);

    //将key标识的对象的指针移除管理map, 同时释放指针指向的对象的资源
    void eraseObject(const string& key);

    //对所有被管理的对象进行功能调用
    using objectFunction = std::function<void(const string& key, T*)>;
    void invokeOnAllObject(objectFunction func);
};




#endif //EXHIBITION_OBJECPTRTHOLDER_H
