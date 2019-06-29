#pragma once

#ifdef MULTICORE
#ifndef __clang__
#include <omp.h>
#endif
#endif

template <typename T, typename F>
bool OmpExecuteBool(std::vector<T>& items, F& f) {
  int failed = 0;
#ifdef MULTICORE
#ifndef __clang__
#pragma omp parallel for
#endif
#endif
  for (size_t i = 0; i < items.size(); ++i) {
    if (failed) continue;
    auto& item = items[i];
    if (!f(item)) {
#ifdef MULTICORE
#pragma omp atomic
#endif
      ++failed;
    }
  }

  return failed == 0;
}

template <typename T, typename F>
void OmpExecuteVoid(std::vector<T>& items, F& f) {
#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (size_t i = 0; i < items.size(); ++i) {
    auto& item = items[i];
    f(item);
  }
}