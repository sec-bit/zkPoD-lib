#include "scheme_ot_vrfq_bob.h"
#include "misc.h"
#include "public.h"
#include "scheme_ot_vrfq_notary.h"
#include "scheme_table.h"
#include "scheme_table_bob_data.h"

namespace scheme::table::ot_vrfq {

Bob::Bob(BobDataPtr b, h256_t const& self_id, h256_t const& peer_id,
         std::string const& query_key,
         std::vector<std::string> const& query_values,
         std::vector<std::string> const& phantoms)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      query_key_(query_key),
      query_values_(query_values),
      phantoms_count_(phantoms.size()) {
  if (phantoms.empty()) throw std::runtime_error("invalid phantoms");
  vrf_key_ = GetKeyMetaByName(b_->vrf_meta(), query_key);
  if (!vrf_key_) throw std::runtime_error("invalid query_key");

  CryptoPP::Keccak_256 hash;
  value_digests_.resize(query_values_.size());
  shuffled_value_digests_.resize(query_values_.size() + phantoms.size());
  for (size_t i = 0; i < query_values.size(); ++i) {
    hash.Update((uint8_t*)query_values[i].data(), query_values[i].size());
    hash.Final(value_digests_[i].data());
    shuffled_value_digests_[i] = value_digests_[i];
  }

  // do not need to unique
  for (size_t i = 0; i < phantoms.size(); ++i) {
    hash.Update((uint8_t*)phantoms[i].data(), phantoms[i].size());
    hash.Final(shuffled_value_digests_[i + value_digests_.size()].data());
  }

  std::default_random_engine generator{std::random_device{}()};
  std::shuffle(shuffled_value_digests_.begin(), shuffled_value_digests_.end(),
               generator);
  shuffle_reference_.resize(value_digests_.size());
  for (size_t i = 0; i < shuffle_reference_.size(); ++i) {
    auto it = std::find(shuffled_value_digests_.begin(),
                        shuffled_value_digests_.end(), value_digests_[i]);
    assert(it != shuffled_value_digests_.end());
    shuffle_reference_[i] = std::distance(shuffled_value_digests_.begin(), it);
  }

  ot_self_pk_ = G1Rand();
  ot_beta_ = FrRand();
  ot_rand_a_ = FrRand();
  ot_rand_b_ = FrRand();
}

void Bob::GetNegoReqeust(NegoBRequest& request) { request.t = ot_self_pk_; }

bool Bob::OnNegoRequest(NegoARequest const& request, NegoAResponse& response) {
  ot_peer_pk_ = request.s;
  response.s_exp_beta = ot_peer_pk_ * ot_beta_;
  return true;
}

bool Bob::OnNegoResponse(NegoBResponse const& response) {
  ot_sk_ = response.t_exp_alpha * ot_beta_;
  return true;
}

void Bob::GetRequest(Request& request) {
  request.key_name = query_key_;
  request.shuffled_value_digests = std::move(shuffled_value_digests_);

  request.ot_vi.resize(value_digests_.size());
  for (size_t i = 0; i < request.ot_vi.size(); ++i) {
    h256_t const& key_digest = value_digests_[i];
    Fr key_fr = BinToFr31(key_digest.data(), key_digest.data() + 31);
    request.ot_vi[i] = ot_sk_ * (ot_rand_b_ * key_fr);
  }
  request.ot_v = ot_self_pk_ * (ot_rand_a_ * ot_rand_b_);
}

bool Bob::OnResponse(Response const& response, Receipt& receipt) {
  if (response.ot_ui.size() != value_digests_.size()) {
    assert(false);
    return false;
  }
  if (response.shuffled_psk_exp_r.size() !=
      (phantoms_count_ + value_digests_.size())) {
    assert(false);
    return false;
  }

  std::vector<vrf::Psk<>> psk_exp_r(value_digests_.size());
  for (size_t i = 0; i < shuffle_reference_.size(); ++i) {
    psk_exp_r[i] = response.shuffled_psk_exp_r[shuffle_reference_[i]];
  }

  last_psk_exp_r_.resize(psk_exp_r.size());
  for (size_t i = 0; i < psk_exp_r.size(); ++i) {
    Fp12 e;
    G1 ui_exp_a = response.ot_ui[i] * ot_rand_a_;
    mcl::bn256::pairing(e, ui_exp_a, ot_peer_pk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    if (ret_len != sizeof(buf)) {
      assert(false);
      throw std::runtime_error("oops");
    }
    G1 ge = MapToG1(buf, sizeof(buf));
    for (auto& j : psk_exp_r[i]) {
      j -= ge;
    }

    if (!vrf::VerifyWithR(b_->vrf_pk(), value_digests_[i].data(), psk_exp_r[i],
                          response.g_exp_r)) {
      assert(false);
      return false;
    }
    last_psk_exp_r_[i] = psk_exp_r[i].back();
  }

  g_exp_r_ = response.g_exp_r;
  receipt.g_exp_r = response.g_exp_r;
  return true;
}

bool Bob::OnSecret(Secret const& query_secret,
                   std::vector<std::vector<uint64_t>>& positions) {
  if (!VerifyProof(g_exp_r_, query_secret)) {
    assert(false);
    return false;
  }

  positions.resize(last_psk_exp_r_.size());
  fsk_.resize(last_psk_exp_r_.size());
  for (size_t i = 0; i < last_psk_exp_r_.size(); ++i) {
    vrf::GetFskFromPskExpR(last_psk_exp_r_[i], query_secret.r, fsk_[i]);

    uint8_t fsk_bin[12 * 32];
    fsk_[i].serialize(fsk_bin, sizeof(fsk_bin), mcl::IoMode::IoSerialize);
    Fr fr_fsk = MapToFr(fsk_bin, sizeof(fsk_bin));

    auto const& key_m = b_->key_m();
    auto const& km = key_m[vrf_key_->j];

    auto& postion = positions[i];
    for (uint64_t j = 0; j < km.size(); ++j) {
      if (fr_fsk == km[j]) {
        postion.push_back(j);
        if (vrf_key_->unique) break;
      }
    }
  }

  return true;
}
}  // namespace scheme::table::ot_vrfq