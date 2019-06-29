#include "misc.h"
#include "scheme_atomic_swap_vc_misc.h"
#include "scheme_misc.h"
#include "tick.h"

namespace scheme::atomic_swap_vc {

template <typename BobData>
Bob<BobData>::Bob(std::shared_ptr<BobData> b, h256_t const& self_id,
                  h256_t const& peer_id, std::vector<Range> demands)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demands_(std::move(demands)) {
  zk_vk_ = ZkpKey::instance().GetZkVk("atomic_swap_vc");
  if (!zk_vk_) {
    assert(false);
    throw std::invalid_argument("missing atomic_swap_vc.vk");
  }

  if (!CheckDemands(n_, demands_)) {
    assert(false);
    throw std::invalid_argument("invalid demands");
  }

  for (auto const& i : demands_) {
    demands_count_ += i.count;
  }

  BuildMapping();

  seed2_seed_ = misc::RandH256();
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
      ++index;
    }
  }
}

template <typename BobData>
void Bob<BobData>::GetRequest(Request& request) {
  request.seed2_seed = seed2_seed_;
  request.demands = demands_;
}

template <typename BobData>
bool Bob<BobData>::OnResponse(Response response, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);

  auto pad_count_of_col = PadCountOfCol(demands_count_);
  if (response.k.size() != (demands_count_ + pad_count_of_col) * s_) {
    assert(false);
    return false;
  }
  if (response.m.size() != demands_count_ * s_) {
    assert(false);
    return false;
  }
  if (response.vw.size() != s_) {
    assert(false);
    return false;
  }

  k_ = std::move(response.k);

  vw_ = std::move(response.vw);

  std::vector<h256_t> seed2_h{{self_id_, peer_id_, seed2_seed_,
                               CalcRangesDigest(demands_), CalcRootOfK(k_)}};
  seed2_ = CalcSeed2(seed2_h);

  ChainKeccak256(seed2_, demands_count_, w_);

  encrypted_m_ = std::move(response.m);

  if (!CheckEncryptedM()) {
    assert(false);
    return false;
  }

  if (!CheckKVW()) {
    assert(false);
    return false;
  }

  sigma_vw_ = FrZero();
  for (auto const& i : vw_) {
    sigma_vw_ += i;
  }

  seed0_mimc3_digest_ = response.seed0_mimc3_digest;

  std::vector<ZkItem> zk_items;
  BuildZkItems(zk_items, demands_count_, s_, w_);
  if (response.zk_ip_vw.size() != zk_items.size()) {
    assert(false);
    std::cerr << "ASSERT: " << __FUNCTION__ << ": " << __LINE__ << "\n";
    return false;
  }

  // Do not need to every zk_ip_vw, just check the accumulate is OK.
  auto const& zk_ip_vw = response.zk_ip_vw;  
  if (std::accumulate(zk_ip_vw.begin(), zk_ip_vw.end(), FrZero()) !=
      sigma_vw_) {
    assert(false);
    std::cerr << "ASSERT: " << __FUNCTION__ << ": " << __LINE__ << "\n";
    return false;
  }

  auto const& zk_proofs = response.zk_proofs;
  if (!VerifyZkProofs(zk_proofs, *zk_vk_, zk_items, response.zk_ip_vw,
                      seed0_mimc3_digest_)) {
    assert(false);
    std::cerr << "ASSERT: " << __FUNCTION__ << ": " << __LINE__ << "\n";
    return false;
  }

  receipt.seed0_mimc3_digest = seed0_mimc3_digest_;

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
    G1 left = sigma * w_[i];
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      left += k_[is + j];
    }
    G1 right = G1Zero();
    for (uint64_t j = 0; j < s_; ++j) {
      Fr const& m = encrypted_m_[is + j];
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
    std::cerr << "ASSERT: " << __FUNCTION__ << ": " << __LINE__ << "\n";
    return false;
  }
  return true;
}

template <typename BobData>
bool Bob<BobData>::CheckKVW() {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  G1 right = G1Zero();
  for (size_t j = 0; j < s_; ++j) {
    right += ecc_pub.PowerU1(j, vw_[j]);
  }

  G1 left = G1Zero();
  size_t offset = demands_count_ * s_;
  for (size_t i = offset; i < k_.size(); ++i) {
    left += k_[i];
  }

  std::vector<G1> sigma_k(demands_count_);
  for (size_t i = 0; i < demands_count_; ++i) {
    sigma_k[i] = G1Zero();
    for (size_t j = 0; j < s_; ++j) {
      sigma_k[i] += k_[i * s_ + j];
    }
  }

  left += MultiExpBdlo12(sigma_k, w_);

  if (right != left) {
    assert(false);
    std::cerr << "ASSERT: " << __FUNCTION__ << ": " << __LINE__ << "\n";
    return false;
  }
  return true;
}

template <typename BobData>
bool Bob<BobData>::OnSecret(Secret const& secret) {
  Tick _tick_(__FUNCTION__);

  if (!VerifyProof(secret.seed0, secret.seed0_rand, seed0_mimc3_digest_)) {
    assert(false);
    std::cerr << "ASSERT: " << __FUNCTION__ << ": " << __LINE__ << "\n";
    return false;
  }

  // compute v
  std::vector<Fr> v;
  auto pad_count_of_col = PadCountOfCol(demands_count_);
  ChainMimcInv(secret.seed0, (demands_count_ + pad_count_of_col) * s_, v);

#ifdef _DEBUG
  Fr check_sigma_vw = FrZero();
  size_t offset = demands_count_ * s_;
  for (size_t i = offset; i < v.size(); ++i) {
    check_sigma_vw += v[i];
  }

  for (size_t i = 0; i < demands_count_; ++i) {
    Fr sigma_v = FrZero();
    for (size_t j = 0; j < s_; ++j) {
      sigma_v += v[i * s_ + j];
    }
    check_sigma_vw += sigma_v * w_[i];
  }
  assert(check_sigma_vw == sigma_vw_);
#endif

  DecryptM(v);

  return true;
}

template <typename BobData>
void Bob<BobData>::DecryptM(std::vector<Fr> const& v) {
  Tick _tick_(__FUNCTION__);

  std::vector<Fr> inv_w = w_;
  FrInv(inv_w.data(), inv_w.size());

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto is = i * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      encrypted_m_[ij] = (encrypted_m_[ij] - v[ij]) * inv_w[i];
    }
  }

  decrypted_m_ = std::move(encrypted_m_);
}

template <typename BobData>
bool Bob<BobData>::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);
  return b_->SaveDecryped(file, demands_, decrypted_m_);
}
}  // namespace scheme::atomic_swap_vc
