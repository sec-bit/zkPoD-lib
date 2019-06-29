#include "scheme_atomic_swap_vc_notary.h"
#include "chain.h"
#include "mkl_tree.h"
#include "scheme_misc.h"
#include "scheme_atomic_swap_vc_misc.h"
#include "mimc.h"

namespace scheme::atomic_swap_vc {
bool VerifyProof(Fr const& seed0, Fr const& seed0_rand, Fr const& digest) {
  auto check_digest = Mimc3(seed0, seed0_rand);
  return check_digest == digest;
}
}  // namespace scheme::atomic_swap_vc
