#pragma once

#include <string>
#include <vector>
#include "ecc.h"
#include "zkp.h"

namespace scheme::atomic_swap_vc {
struct Request {
  h256_t seed2_seed;
  std::vector<Range> demands;
};

struct Response {
  std::vector<G1> k;   // (n+1)*s
  std::vector<Fr> m;   // n*s
  std::vector<Fr> vw;  // s
  
  Fr seed0_mimc3_digest;
  std::vector<Fr> zk_ip_vw;
  std::vector<ZkProof> zk_proofs;
};

struct Receipt {
  Fr seed0_mimc3_digest;
};

struct Secret {
  Fr seed0;
  Fr seed0_rand;
};
}  // namespace scheme::atomic_swap_vc
