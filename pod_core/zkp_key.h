#pragma once

#include <boost/noncopyable.hpp>
#include <mutex>
#include <unordered_map>
#include "zkp.h"
#include "tick.h"

class ZkpKey : boost::noncopyable {
 public:
  static ZkpKey& instance(std::string const& path = "") {
    static ZkpKey _instance_(path);
    return _instance_;
  }

  ZkPkPtr GetZkPk(std::string const& name) const {
    auto it = pk_.find(name);
    if (it != pk_.end()) return it->second;
    return ZkPkPtr();
  }

  ZkVkPtr GetZkVk(std::string const& name) const {
    auto it = vk_.find(name);
    if (it != vk_.end()) return it->second;
    return ZkVkPtr();
  }

  bool IsEmpty() const { return pk_.empty() && vk_.empty(); }

 private:
  ZkpKey(std::string const& path) : path_(path) {
    Tick tick(__FUNCTION__);
    auto range = boost::make_iterator_range(fs::directory_iterator(path_), {});
    for (auto& entry : range) {
      auto basename = fs::basename(entry);
      auto extension = fs::extension(entry);
      auto fullpath = entry.path().string();
      if (extension == ".vk") {
        auto vk = LoadZkVk(fullpath);
        if (vk) vk_[basename] = vk;
      } else if (extension == ".pk") {
        auto pk = LoadZkPk(fullpath);
        if (pk) pk_[basename] = pk;
      }
    }
  }
  std::string path_;
  std::unordered_map<std::string, ZkPkPtr> pk_;
  std::unordered_map<std::string, ZkVkPtr> vk_;
};