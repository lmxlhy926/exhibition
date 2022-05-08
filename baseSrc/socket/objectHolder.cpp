//
// Created by 78472 on 2022/5/8.
//

#include "objectHolder.h"

template<typename T>
objectHolder<T>::~objectHolder() {

}

template<typename T>
void objectHolder<T>::appendNew(const string &key, T *objPtr) {

}

template<typename T>
T *objectHolder<T>::findObject(const string &key) {
    return nullptr;
}

template<typename T>
bool objectHolder<T>::existObject(const string &key) {
    return false;
}

template<typename T>
void objectHolder<T>::eraseObject(const string &key) {

}

template<typename T>
void objectHolder<T>::invokeOnAllObject(objectHolder::objectFunction func) {

}