#pragma once

#include "omp_helper.h"
#include "public.h"
#include "basic_types.h"
#include "ecc.h"
#include "scheme_batch2_protocol.h"
#include "scheme_batch2_notary.h"
#include "scheme_misc.h"

namespace scheme::batch2 {

template<typename B>
class Client {
 public:
  // The self_id and peer_id are useless now, just for later convenience.
  Client(std::shared_ptr<B> b, h256_t const& self_id, h256_t const& peer_id,
         std::vector<Range> demands);

 public:
  void GetRequest(Request& request);
  bool OnResponse(Response response, Receipt& receipt);
  bool OnSecret(Secret const& secret);
  bool SaveDecrypted(std::string const& file);

 private:
  void BuildMapping();
  bool CheckEncryptedM();
  bool CheckKVW();
  void DecryptM(std::vector<Fr> const& v);

 private:
  std::shared_ptr<B> b_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;
  std::vector<Range> const demands_;
  uint64_t demands_count_ = 0;
  h256_t seed2_seed_;

 private:
  std::vector<G1> k_;   // sizeof() = (count + 1) * s
  std::vector<Fr> vw_;  // sizeof() = s

 private:
  struct Mapping {
    uint64_t global_index;
  };
  std::vector<Mapping> mappings_;
  Fr sigma_vw_;

 private:
  h256_t seed2_;
  std::vector<Fr> w_;  // size() = count
  std::vector<Fr> decrypted_m_;
  std::vector<Fr> encrypted_m_;
};

template <typename B>
using ClientPtr = std::shared_ptr<Client<B>>;
}  // namespace scheme::table::batch2

#include "scheme_batch2_client.inc"