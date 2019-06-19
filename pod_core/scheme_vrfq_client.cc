#include "scheme_vrfq_client.h"
#include "public.h"
#include "scheme_table.h"
#include "scheme_table_bob_data.h"
#include "scheme_vrfq_notary.h"

namespace scheme::table::vrfq {

Client::Client(BobDataPtr b, h256_t const& self_id, h256_t const& peer_id,
               std::string const& query_key,
               std::vector<std::string> const& query_values)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      query_key_(query_key),
      query_values_(query_values) {
  if (query_key_.empty()) throw std::invalid_argument("query_key empty");
  if (query_values_.empty()) throw std::invalid_argument("query_values empty");
  for (auto const& i : query_values_) {
    if (i.empty()) throw std::invalid_argument("query_value empty");
  }

  vrf_key_ = GetKeyMetaByName(b_->vrf_meta(), query_key);
  if (!vrf_key_) throw std::runtime_error("invalid query_key");

  CryptoPP::Keccak_256 hash;
  value_digests_.resize(query_values_.size());
  for (size_t i = 0; i < query_values_.size(); ++i) {
    auto const& value = query_values_[i];
    hash.Update((uint8_t*)value.data(), value.size());
    hash.Final(value_digests_[i].data());
  }

  last_psk_exp_r_.resize(query_values.size());
  fsk_.resize(query_values.size());
}

void Client::GetRequest(Request& request) {
  request.key_name = query_key_;
  request.value_digests = value_digests_;
}

bool Client::OnResponse(Response const& response, Receipt& receipt) {
  if (response.psk_exp_r.size() != query_values_.size()) {
    assert(false);
    return false;
  }

  g_exp_r_ = response.g_exp_r;
  for (size_t i = 0; i < response.psk_exp_r.size(); ++i) {
    auto const& key_digest = value_digests_[i];
    auto const& psk_exp_r = response.psk_exp_r[i];
    if (!vrf::VerifyWithR(b_->vrf_pk(), key_digest.data(), psk_exp_r,
                          g_exp_r_)) {
      assert(false);
      return false;
    }

    last_psk_exp_r_[i] = psk_exp_r.back();
    receipt.g_exp_r = g_exp_r_;
  }

  return true;
}

bool Client::OnSecret(Secret const& query_secret,
                      std::vector<std::vector<uint64_t>>& positions) {
  if (!VerifyProof(g_exp_r_, query_secret)) {
    assert(false);
    return false;
  }

  positions.resize(query_values_.size());
  for (size_t i = 0; i < last_psk_exp_r_.size(); ++i) {
    vrf::GetFskFromPskExpR(last_psk_exp_r_[i], query_secret.r, fsk_[i]);

    uint8_t fsk_bin[12 * 32];
    fsk_[i].serialize(fsk_bin, sizeof(fsk_bin), mcl::IoMode::IoSerialize);

    CryptoPP::Keccak_256 hash;
    h256_t digest;
    hash.Update(fsk_bin, sizeof(fsk_bin));
    hash.Final(digest.data());
    Fr fr_fsk = BinToFr31(digest.data(), digest.data() + 31);

    auto const& key_m = b_->key_m();
    auto const& km = key_m[vrf_key_->j];

    auto& position = positions[i];
    for (uint64_t j = 0; j < km.size(); ++j) {
      if (fr_fsk == km[j]) {
        position.push_back(j);
        if (vrf_key_->unique) break;
      }
    }
  }

  return true;
}
}  // namespace scheme::table::vrfq