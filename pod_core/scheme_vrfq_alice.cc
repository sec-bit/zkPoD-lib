#include "scheme_vrfq_alice.h"
#include "public.h"
#include "scheme_table_alice_data.h"
#include "vrf.h"

namespace scheme::table::vrfq {

Alice::Alice(AliceDataPtr a, h256_t const& self_id, h256_t const& peer_id)
    : a_(a), self_id_(self_id), peer_id_(peer_id) {
  auto const& ecc_pub = GetEccPub();
  r_ = FrRand();
  g_exp_r_ = ecc_pub.PowerG1(r_);
}

bool Alice::OnRequest(Request const& request, Response& response) {
  if (request.value_digests.empty()) return false;
  auto key_meta = a_->GetKeyMetaByName(request.key_name);
  if (!key_meta) return false;

  response.g_exp_r = g_exp_r_;
  response.psk_exp_r.resize(request.value_digests.size());
  for (size_t i = 0; i < request.value_digests.size(); ++i) {
    auto const& key_digest = request.value_digests[i];
    auto& psk_exp_r = response.psk_exp_r[i];
    vrf::ProveWithR(a_->vrf_sk(), key_digest.data(), r_, psk_exp_r);

#ifdef _DEBUG
    vrf::Fsk fsk2 = vrf::Vrf(a_->vrf_sk(), key_digest.data());
    vrf::VerifyWithR(a_->vrf_pk(), key_digest.data(), psk_exp_r,
                     response.g_exp_r);
    vrf::Fsk fsk1;
    vrf::GetFskFromPskExpR(psk_exp_r.back(), r_, fsk1);
    assert(fsk1 == fsk2);
#endif
  }
  return true;
}

bool Alice::OnReceipt(Receipt const& receipt, Secret& secret) {
  if (receipt.g_exp_r != g_exp_r_) return false;
  secret.r = r_;
  return true;
}
}  // namespace scheme::table::vrfq
