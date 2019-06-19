#pragma once

#include <string>
#include <vector>

#include "ecc.h"
#include "vrf.h"

namespace scheme::table::vrfq {
struct Request {
  std::string key_name;
  std::vector<h256_t> value_digests;
};

struct Response {
  std::vector<vrf::Psk<>> psk_exp_r;
  G1 g_exp_r;
};

struct Receipt {
  G1 g_exp_r;
};

struct Secret {
  Fr r;
};
}  // namespace scheme::table::vrfq