#include "scheme_atomic_swap_notary.h"
#include "chain.h"
#include "mkl_tree.h"
#include "scheme_misc.h"

namespace scheme::atomic_swap {
bool VerifyProof(uint64_t s, Receipt const& receipt, Secret const& secret) {
  std::vector<Fr> v;
  ChainKeccak256(secret.seed0, (receipt.count + 1) * s, v);
  std::vector<Fr> w;
  ChainKeccak256(receipt.seed2, receipt.count, w);

  return VerifyProof(s, receipt.count, receipt.sigma_vw, v, w);
}

bool VerifyProof(uint64_t s, uint64_t count, Fr const& sigma_vw,
                 std::vector<Fr> const& v, std::vector<Fr> const& w) {
  assert(v.size() == (count + 1) * s);
  assert(w.size() == count);
  Fr check_sigma_vw = FrZero();

  size_t offset = count * s;
  for (size_t j = 0; j < s; ++j) {
    check_sigma_vw += v[offset + j];
  }
  for (size_t i = 0; i < count; ++i) {
    Fr sigma_v = FrZero();
    for (size_t j = 0; j < s; ++j) {
      sigma_v += v[i * s + j];
    }
    check_sigma_vw += sigma_v * w[i];
  }
  return check_sigma_vw == sigma_vw;
}
}  // namespace scheme::atomic_swap
