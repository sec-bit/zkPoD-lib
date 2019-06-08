#pragma once

#ifndef __clang__
#include <omp.h>
#endif

template <typename T, typename F>
bool OmpExecuteBool(std::vector<T>& items, F& f) {
  int failed = 0;
#pragma omp parallel for
  for (size_t i = 0; i < items.size(); ++i) {
    if (failed) continue;
    auto& item = items[i];
    if (!f(item)) {
#pragma omp atomic
      ++failed;
    }
  }

  return failed == 0;
}

template <typename T, typename F>
void OmpExecuteVoid(std::vector<T>& items, F& f) { 
#pragma omp parallel for
  for (size_t i = 0; i < items.size(); ++i) {
    auto& item = items[i];
    f(item);
  }
}