#pragma once

#include <iostream>
#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc_pub.h"

namespace scheme::otbatch3 {
template <typename Receipt, typename Secret>
bool VerifyProof(Receipt const& receipt, Secret const& secret) {
  Tick _tick_(__FUNCTION__);
  // NOTE: Blockchain vm does not have ecc pub, must call u^v directly
  auto const& ecc_pub = GetEccPub();
  if (receipt.u0d != ecc_pub.PowerU1(0, secret.d)) {
    assert(false);
    std::cerr << __FUNCTION__ << ":" << __LINE__ << " ASSERT\n";
    return false;
  }
  if (receipt.u0_x0_lgs != ecc_pub.PowerU1(0, secret.x0_lgs)) {
    assert(false);
    std::cerr << __FUNCTION__ << ":" << __LINE__ << " ASSERT\n";
    std::cout << "receipt.u0_x0_lgs: " << receipt.u0_x0_lgs << "\n";
    std::cout << "secret.x0_lgs: " << secret.x0_lgs << "\n";
    return false;
  }
  return true;
}
}  // namespace scheme::otbatch3