#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "vrf.h"

namespace scheme::table::ot_vrfq {
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
  std::string key_name;
  std::vector<h256_t> shuffled_value_digests;  // sizeof() = L
  std::vector<G1> ot_vi;                       // sizeof() = K
  G1 ot_v;
};

struct Response {
  std::vector<vrf::Psk<>> shuffled_psk_exp_r;  // sizeof() = L
  G1 g_exp_r;
  std::vector<G1> ot_ui;  // sizeof() = K
};

struct Receipt {
  G1 g_exp_r;
};

struct Secret {
  Fr r;
};
}  // namespace scheme::table::ot_vrfq
