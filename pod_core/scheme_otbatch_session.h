#pragma once

#include "ecc.h"
#include "public.h"
#include "scheme_otbatch_protocol.h"

namespace scheme::otbatch {

template <typename A>
class Session {
 public:
  typedef std::shared_ptr<A> APtr;
  // The self_id and peer_id are useless now, just for later convenience.
  Session(APtr a, h256_t const& self_id, h256_t const& peer_id);

 public:
  void GetNegoReqeust(NegoARequest& request);
  bool OnNegoRequest(NegoBRequest const& request, NegoBResponse& response);
  bool OnNegoResponse(NegoAResponse const& response);

 public:
  bool OnRequest(Request request, Response& response);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 public:
  void TestSetEvil() { evil_ = true; }

 private:
  void BuildMapping();

 private:
  APtr a_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;

 private:
  std::vector<Range> phantoms_;  // sizeof() = L
  std::vector<G1> ot_vi_;        // sizeof() = K
  G1 ot_v_;
  h256_t seed2_;
  h256_t seed2_seed_;

 private:
  struct Mapping {
    uint64_t global_index;
  };
  uint64_t phantoms_count_ = 0;
  std::vector<Mapping> mappings_;

 private:
  h256_t seed0_;
  std::vector<Fr> v_;  // size() is count * s_
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;

 private:
  G2 ot_self_pk_;
  G1 ot_peer_pk_;
  G2 ot_sk_;
  Fr ot_alpha_;
  Fr ot_rand_c_;

 private:
  bool evil_ = false;
};

template <typename A>
using SessionPtr = std::shared_ptr<Session<A>>;

}  // namespace scheme::otbatch

#include "scheme_otbatch_session.inc"