#pragma once

#include "ecc.h"
#include "public.h"
#include "scheme_complaint_protocol.h"

namespace scheme::complaint {

template <typename AliceData>
class Alice {
 public:
  Alice(std::shared_ptr<AliceData> a, h256_t const& self_id,
        h256_t const& peer_id);

 public:
  bool OnRequest(Request request, Response& response);
  bool OnReceipt(Receipt const& receipt, Secret& secret);

 public:
  void TestSetEvil() { evil_ = true; }

 private:
  void BuildMapping();

 private:
  std::shared_ptr<AliceData> a_;
  h256_t const self_id_;
  h256_t const peer_id_;
  uint64_t const n_;
  uint64_t const s_;

 private:
  std::vector<Range> demands_;
  h256_t seed2_seed_;
  h256_t seed2_;

 private:
  struct Mapping {
    uint64_t global_index;
  };
  uint64_t demands_count_ = 0;
  std::vector<Mapping> mappings_;

 private:
  h256_t seed0_;
  std::vector<Fr> v_;  // size() is count * s_
  std::vector<Fr> w_;  // size() is count
  h256_t k_mkl_root_;

 private:
  bool evil_ = false;
};

template <typename AliceData>
using AlicePtr = std::shared_ptr<Alice<AliceData>>;

}  // namespace scheme::complaint

#include "scheme_complaint_alice.inc"