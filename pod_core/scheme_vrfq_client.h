#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "ecc.h"
#include "scheme_table_bob_data.h"
#include "scheme_vrfq_protocol.h"
#include "vrf.h"

namespace scheme::table {

struct VrfKeyMeta;

namespace vrfq {
class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(BobDataPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::string const& query_key,
         std::vector<std::string> const& query_values);

 public:
  void GetRequest(Request& request);
  bool OnResponse(Response const& request, Receipt& receipt);
  bool OnSecret(Secret const& secret,
                std::vector<std::vector<uint64_t>>& positions);

 private:
  BobDataPtr b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  std::string const query_key_;
  std::vector<std::string> const query_values_;

 private:
  VrfKeyMeta const* vrf_key_;
  std::vector<h256_t> value_digests_;
  std::vector<G1> last_psk_exp_r_;
  std::vector<vrf::Fsk> fsk_;
  G1 g_exp_r_;
};

typedef std::shared_ptr<Client> ClientPtr;
}  // namespace vrfq

}  // namespace scheme::table