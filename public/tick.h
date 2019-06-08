#pragma once

#include <chrono>
#include <iostream>
#include <string>

struct Tick {
  Tick(std::string const& desc) : desc_(desc) {
    start_ = std::chrono::steady_clock::now();
    std::cout << "==> " << desc_ << "\n";
  }
  ~Tick() {
    auto t = std::chrono::steady_clock::now() - start_;
    auto s = std::chrono::duration_cast<std::chrono::seconds>(t);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t);
    if (s.count() < 10) {
      std::cout << "<== " << desc_ << " tick: " << ms.count() << " ms\n";
    } else {
      std::cout << "<== " << desc_ << " tick: " << s.count() << " seconds\n";
    }
  }
  std::string desc_;
  std::chrono::steady_clock::time_point start_;
};