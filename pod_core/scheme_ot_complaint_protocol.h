#pragma once

#include <string>
#include <vector>

#include "basic_types.h"
#include "ecc.h"

namespace scheme::ot_complaint {
struct NegoARequest {
  G2 s;
};

struct NegoAResponse {
  G2 s_exp_beta;
};

struct NegoBRequest {
  G1 t;
};

struct NegoBResponse {
  G1 t_exp_alpha;
};

struct Request {
  h256_t seed2_seed;
  std::vector<Range> phantoms;  // sizeof() = L
  std::vector<G1> ot_vi;        // sizeof() = K
  G1 ot_v;
};

struct Response {
  std::vector<G1> k;      // sizeof() = L
  std::vector<G1> ot_ui;  // sizeof() = K
  std::vector<Fr> m;      // sizeof() = L
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
}  // namespace scheme::ot_complaint
