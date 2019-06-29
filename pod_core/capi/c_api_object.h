#pragma once

#include <memory>
#include <unordered_map>

template <typename T>
struct CapiObject {
  static void Add(T* p) {
    std::shared_ptr<T> ptr(p);
    std::scoped_lock<std::mutex> lock(mutex);
    datas.insert(std::make_pair(p, ptr));
  }

  static std::shared_ptr<T> Get(handle_t h) {
    std::shared_ptr<T> ptr;
    std::scoped_lock<std::mutex> lock(mutex);
    auto it = datas.find(h);
    if (it != datas.end()) ptr = it->second;
    return ptr;
  }

  static bool Del(handle_t p) {
    std::scoped_lock<std::mutex> lock(mutex);
    return datas.erase((T*)p) != 0;
  }

 private:
  static std::mutex mutex;
  static std::unordered_map<void*, std::shared_ptr<T>> datas;
};

template <typename T>
std::mutex CapiObject<T>::mutex;

template <typename T>
std::unordered_map<void*, std::shared_ptr<T>> CapiObject<T>::datas;
