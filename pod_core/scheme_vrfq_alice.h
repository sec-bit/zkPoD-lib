#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_table_alice_data.h"
#include "scheme_vrfq_protocol.h"

namespace scheme::table::vrfq {

class Alice {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Alice(AliceDataPtr a, h256_t const& self_id, h256_t const& peer_id);

 public:
  bool OnRequest(Request const& request, Response& response);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 private:
  AliceDataPtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;

 private:
  Fr r_;
  G1 g_exp_r_;
};

typedef std::shared_ptr<Alice> AlicePtr;
}  // namespace scheme::table::vrfq