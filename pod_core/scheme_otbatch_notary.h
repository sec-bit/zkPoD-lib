#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "chain.h"
#include "mkl_tree.h"
#include "scheme_misc.h"

namespace scheme::otbatch {
template <typename Receipt, typename Secret, typename Claim>
bool VerifyClaim(uint64_t s, Receipt const& receipt, Secret const& secret,
                 Claim const& claim) {
  // same with the scheme::plain::range::VerifyClaim
  if (!VerifyPathOfK(claim.kij, claim.i * s + claim.j, receipt.count * s,
                     receipt.k_mkl_root, claim.mkl_path)) {
    assert(false);
    return false;
  }

  // NOTE: Blockchain vm does not have ecc pub, must call u^v directly
  auto const& ecc_pub = GetEccPub();
  Fr v = Chain(secret.seed0, claim.i * s + claim.j);
  G1 check_k = ecc_pub.PowerU1(claim.j, v);
  if (check_k == claim.kij) {
    assert(false);
    return false;
  }

  return true;
}
}  // namespace scheme::otbatch
