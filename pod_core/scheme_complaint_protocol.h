#pragma once

#include <string>
#include <vector>
#include "ecc.h"

namespace scheme::complaint {
struct Request {
  h256_t seed2_seed;
  std::vector<Range> demands;
};

struct Response {
  std::vector<G1> k;
  std::vector<Fr> m;
};

struct Receipt {
  h256_t seed2;
  h256_t k_mkl_root;
  uint64_t count;
};

struct Secret {
  h256_t seed0;
};

struct Claim {
  uint64_t i;
  uint64_t j;
  G1 kij;
  std::vector<h256_t> mkl_path;
};
}  // namespace scheme::complaint
