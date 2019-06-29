#include "misc.h"
#include "scheme_misc.h"
#include "tick.h"

namespace scheme::ot_complaint {

template <typename BobData>
Bob<BobData>::Bob(std::shared_ptr<BobData> b, h256_t const& self_id,
                  h256_t const& peer_id, std::vector<Range> demands,
                  std::vector<Range> phantoms)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demands_(std::move(demands)),
      phantoms_(std::move(phantoms)) {
  if (!CheckDemandPhantoms(n_, demands_, phantoms_)) {
    throw std::invalid_argument("invalid demands or phantoms");
  }
  for (auto const& i : demands_) demands_count_ += i.count;
  for (auto const& i : phantoms_) phantoms_count_ += i.count;
  BuildMapping();

  seed2_seed_ = misc::RandH256();

  ot_self_pk_ = G1Rand();
  ot_beta_ = FrRand();
  ot_rand_a_ = FrRand();
  ot_rand_b_ = FrRand();
}

template <typename BobData>
void Bob<BobData>::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(demands_count_);
  size_t index = 0;
  for (auto const& d : demands_) {
    for (size_t i = d.start; i < (d.start + d.count); ++i) {
      auto& map = mappings_[index];
      map.global_index = i;
      map.phantom_offset = GetRangesOffsetByIndexOfM(phantoms_, i);
      ++index;
    }
  }
}

template <typename BobData>
void Bob<BobData>::GetNegoReqeust(NegoBRequest& request) {
  request.t = ot_self_pk_;
}

template <typename BobData>
bool Bob<BobData>::OnNegoRequest(NegoARequest const& request,
                                 NegoAResponse& response) {
  ot_peer_pk_ = request.s;
  response.s_exp_beta = ot_peer_pk_ * ot_beta_;
  return true;
}

template <typename BobData>
bool Bob<BobData>::OnNegoResponse(NegoBResponse const& response) {
  ot_sk_ = response.t_exp_alpha * ot_beta_;
  return true;
}

template <typename BobData>
void Bob<BobData>::GetRequest(Request& request) {
  request.seed2_seed = seed2_seed_;
  request.phantoms = phantoms_;

  request.ot_vi.reserve(demands_count_);
  for (auto const& i : demands_) {
    for (size_t j = i.start; j < i.start + i.count; ++j) {
      auto fr = MapToFr(j);
      request.ot_vi.push_back(ot_sk_ * (ot_rand_b_ * fr));
    }
  }
  request.ot_v = ot_self_pk_ * (ot_rand_a_ * ot_rand_b_);
  request_ = request;
}

template <typename BobData>
bool Bob<BobData>::OnResponse(Response response, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);
  if (response.k.size() != phantoms_count_ * s_) {
    assert(false);
    return false;
  }
  if (response.ot_ui.size() != demands_count_) {
    assert(false);
    return false;
  }
  if (response.m.size() != phantoms_count_ * s_) {
    assert(false);
    return false;
  }

  k_ = std::move(response.k);
  ot_ui_ = std::move(response.ot_ui);
  k_mkl_root_ = CalcRootOfK(k_);

  std::vector<h256_t> seed2_h{
      {self_id_, peer_id_, seed2_seed_, CalcRangesDigest(phantoms_),
       CalcG1DataDigest(request_.ot_vi), G1ToBin(request_.ot_v),
       CalcG1DataDigest(ot_ui_), k_mkl_root_}};
  seed2_ = CalcSeed2(seed2_h);
  // std::cout << "bob seed: " << misc::HexToStr(seed2_) << "\n";

  ChainKeccak256(seed2_, phantoms_count_, w_);

  encrypted_m_.resize(demands_count_ * s_);

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (int64_t i = 0; i < (int64_t)ot_ui_.size(); ++i) {
    Fp12 e;
    G1 ui_exp_a = ot_ui_[i] * ot_rand_a_;
    mcl::bn256::pairing(e, ui_exp_a, ot_peer_pk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    if (ret_len != sizeof(buf)) {
      assert(false);
      throw std::runtime_error("oops");
    }
    Fr fr_e = MapToFr(buf, sizeof(buf));

    auto phantom_offset = mappings_[i].phantom_offset;
    for (size_t j = 0; j < s_; ++j) {
      encrypted_m_[i * s_ + j] = response.m[phantom_offset * s_ + j] - fr_e;
    }
  }

  if (!CheckEncryptedM()) {
    assert(false);
    return false;
  }

  receipt.count = phantoms_count_;
  receipt.k_mkl_root = k_mkl_root_;
  receipt.seed2 = seed2_;
  return true;
}

template <typename BobData>
bool Bob<BobData>::CheckEncryptedM() {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  auto const& sigmas = b_->sigmas();

  // uint64_t phantom_offset = phantom_.start - demand_.start;
  int not_equal = 0;

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    if (not_equal) continue;
    auto const& mapping = mappings_[i];
    G1 const& sigma = sigmas[mapping.global_index];
    G1 left = sigma * w_[mapping.phantom_offset];
    for (uint64_t j = 0; j < s_; ++j) {
      left += k_[mapping.phantom_offset * s_ + j];
    }
    G1 right = G1Zero();
    for (uint64_t j = 0; j < s_; ++j) {
      Fr const& m = encrypted_m_[i * s_ + j];
      right += ecc_pub.PowerU1(j, m);
    }
    if (left != right) {
#ifdef MULTICORE
#pragma omp atomic
#endif
      ++not_equal;
    }
  }

  if (not_equal) {
    assert(false);
    return false;
  }
  return true;
}

template <typename BobData>
bool Bob<BobData>::OnSecret(Secret const& secret) {
  Tick _tick_(__FUNCTION__);

  // compute v
  std::vector<Fr> v;
  ChainKeccak256(secret.seed0, phantoms_count_ * s_, v);

  if (!CheckK(v)) {
    assert(claim_i_ >= 0 && claim_j_ >= 0);
    return false;
  } else {
    DecryptM(v);
    return true;
  }
}

template <typename BobData>
bool Bob<BobData>::GenerateClaim(Claim& claim) {
  if (claim_i_ == -1 || claim_j_ == -1) {
    assert(false);
    return false;
  }
  BuildClaim(claim_i_, claim_j_, claim);
  return true;
}

template <typename BobData>
bool Bob<BobData>::CheckK(std::vector<Fr> const& v) {
#ifdef MULTICORE
  auto omp_threads = omp_get_max_threads();
#else
  auto omp_threads = 1;
#endif

  if (v.size() > (1024 * 1024) && omp_threads < 3) {
    return CheckKMultiExp(v);
  } else {
    return CheckKDirect(v);
  }
}

template <typename BobData>
bool Bob<BobData>::CheckKDirect(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

  // compute k
  std::vector<G1> k;
  BuildK(v, k, s_);

  // compare k
  for (uint64_t i = 0; i < phantoms_count_; ++i) {
    for (uint64_t j = 0; j < s_; ++j) {
      auto offset = i * s_ + j;
      if (k[offset] == k_[offset]) continue;
      claim_i_ = i;
      claim_j_ = j;
      return false;
    }
  }
  return true;
}

template <typename BobData>
bool Bob<BobData>::CheckKMultiExp(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  uint64_t mismatch_j = (uint64_t)(-1);
  for (uint64_t j = 0; j < s_; ++j) {
    Fr sigma_vij = FrZero();
    std::vector<G1 const*> k(phantoms_count_);
    for (uint64_t i = 0; i < phantoms_count_; ++i) {
      sigma_vij += v[i * s_ + j] * w_[i];
      k[i] = &k_[i * s_ + j];
    }

    G1 check_sigma_kij = ecc_pub.PowerU1(j, sigma_vij);
    G1 sigma_kij = MultiExpBdlo12(k, w_, 0, phantoms_count_);
    if (check_sigma_kij != sigma_kij) {
      mismatch_j = j;
      break;
    }
  }

  if (mismatch_j == (uint64_t)(-1)) return true;

  std::vector<G1 const*> k_col(phantoms_count_);
  std::vector<Fr const*> v_col(phantoms_count_);
  for (uint64_t i = 0; i < phantoms_count_; ++i) {
    auto offset = i * s_ + mismatch_j;
    k_col[i] = &k_[offset];
    v_col[i] = &v[offset];
  }

  uint64_t mismatch_i = FindMismatchI(mismatch_j, k_col, v_col);
  if (mismatch_i == (uint64_t)(-1)) {
    assert(false);
    throw std::runtime_error("oops! FindMismatchI failed to find mismatch i");
  }

  claim_i_ = mismatch_i;
  claim_j_ = mismatch_j;

  return false;
}

template <typename BobData>
void Bob<BobData>::BuildClaim(uint64_t i, uint64_t j, Claim& claim) {
  Tick _tick_(__FUNCTION__);
  claim.i = i;
  claim.j = j;
  auto ij = i * s_ + j;
  claim.kij = k_[ij];
  auto root = CalcPathOfK(k_, ij, claim.mkl_path);
  if (root != k_mkl_root_) {
    assert(false);
    throw std::runtime_error("oops, mkl root mismatch");
  }
}

template <typename BobData>
uint64_t Bob<BobData>::FindMismatchI(uint64_t mismatch_j,
                                     std::vector<G1 const*> const& k_col,
                                     std::vector<Fr const*> const& v_col) {
  Tick _tick_(__FUNCTION__);

  assert(k_col.size() == v_col.size() && !k_col.empty());

  auto const& ecc_pub = GetEccPub();
  uint64_t offset = 0;
  uint64_t count = k_col.size();

  for (;;) {
    if (count == 1) {
      auto check_k = ecc_pub.PowerU1(mismatch_j, *v_col[offset]);
      return (check_k == *k_col[offset]) ? (uint64_t)(-1) : offset;
    }

    uint64_t half_len = count / 2;
    Fr sigma_vij = FrZero();
    for (uint64_t i = 0; i < half_len; ++i) {
      sigma_vij += (*v_col[offset + i]) * w_[offset + i];
    }
    G1 check_sigma_kij = ecc_pub.PowerU1(mismatch_j, sigma_vij);
    G1 sigma_kij = MultiExpBdlo12(k_col, w_, offset, half_len);

    if (check_sigma_kij != sigma_kij) {
      count = half_len;
    } else {
      offset += half_len;
      count -= half_len;
    }
  }
}

template <typename BobData>
void Bob<BobData>::DecryptM(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);
#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto const& mapping = mappings_[i];
    Fr inv_w = FrInv(w_[mapping.phantom_offset]);
    for (uint64_t j = 0; j < s_; ++j) {
      encrypted_m_[i * s_ + j] =
          (encrypted_m_[i * s_ + j] - v[mapping.phantom_offset * s_ + j]) *
          inv_w;
    }
  }

  decrypted_m_ = std::move(encrypted_m_);
}

template <typename BobData>
bool Bob<BobData>::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);
  return b_->SaveDecryped(file, demands_, decrypted_m_);
}
}  // namespace scheme::ot_complaint
