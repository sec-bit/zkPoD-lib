#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ecc.h"
#include "scheme_table_a.h"
#include "scheme_table_vrfq_protocol.h"

namespace scheme::table::vrfq {

class Session {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);

 public:
  bool OnRequest(Request const& request, Response& response);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;

 private:
  Fr r_;
  G1 g_exp_r_;
};

typedef std::shared_ptr<Session> SessionPtr;
}  // namespace scheme::table::vrfq