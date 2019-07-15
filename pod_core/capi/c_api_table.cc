#include "c_api.h"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../public/scheme_table.h"
#include "../scheme_atomic_swap_alice.h"
#include "../scheme_atomic_swap_bob.h"
#include "../scheme_atomic_swap_serialize.h"
#include "../scheme_atomic_swap_vc_alice.h"
#include "../scheme_atomic_swap_vc_bob.h"
#include "../scheme_atomic_swap_vc_serialize.h"
#include "../scheme_complaint_alice.h"
#include "../scheme_complaint_bob.h"
#include "../scheme_complaint_serialize.h"
#include "../scheme_ot_complaint_alice.h"
#include "../scheme_ot_complaint_bob.h"
#include "../scheme_ot_complaint_serialize.h"
#include "../scheme_ot_vrfq_alice.h"
#include "../scheme_ot_vrfq_bob.h"
#include "../scheme_ot_vrfq_serialize.h"
#include "../scheme_table_alice_data.h"
#include "../scheme_table_bob_data.h"
#include "../scheme_vrfq_alice.h"
#include "../scheme_vrfq_bob.h"
#include "../scheme_vrfq_serialize.h"

#include "ecc.h"
#include "ecc_pub.h"

#include "c_api_object.h"

extern "C" {

EXPORT handle_t E_TableAliceDataNew(char const* publish_path) {
  using namespace scheme::table;
  try {
    auto p = new AliceData(publish_path);
    CapiObject<AliceData>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT handle_t E_TableBobDataNew(char const* bulletin_file,
                                  char const* public_path) {
  using namespace scheme::table;
  try {
    auto p = new BobData(bulletin_file, public_path);
    CapiObject<BobData>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableAliceBulletin(handle_t c_alice_data,
                                 table_bulletin_t* bulletin) {
  using namespace scheme::table;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return false;
  Bulletin const& v = alice_data->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  memcpy(bulletin->vrf_meta_digest, v.vrf_meta_digest.data(), 32);
  return true;
}

EXPORT bool E_TableBobBulletin(handle_t c_bob_data,
                               table_bulletin_t* bulletin) {
  using namespace scheme::table;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return false;
  Bulletin const& v = bob_data->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  memcpy(bulletin->vrf_meta_digest, v.vrf_meta_digest.data(), 32);
  return true;
}

EXPORT bool E_TableBIsKeyUnique(handle_t c_bob_data, char const* query_key,
                                bool* unique) {
  using namespace scheme::table;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return false;
  auto vrf_key = GetKeyMetaByName(bob_data->vrf_meta(), query_key);
  if (!vrf_key) return false;
  *unique = vrf_key->unique;
  return true;
}

EXPORT bool E_TableAliceDataFree(handle_t c_alice_data) {
  using namespace scheme::table;
  return CapiObject<AliceData>::Del(c_alice_data);
}

EXPORT bool E_TableBobDataFree(handle_t c_bob_data) {
  using namespace scheme::table;
  return CapiObject<BobData>::Del(c_bob_data);
}

}  // extern "C"

// complaint
extern "C" {
EXPORT handle_t E_TableComplaintAliceNew(handle_t c_alice_data,
                                         uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Alice<AliceData>(alice_data, self_id, peer_id);
    CapiObject<Alice<AliceData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableComplaintAliceOnRequest(handle_t c_alice,
                                           char const* request_file,
                                           char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(request);

    Response response;
    if (!alice->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableComplaintAliceOnReceipt(handle_t c_alice,
                                           char const* receipt_file,
                                           char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!alice->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableComplaintAliceSetEvil(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_TableComplaintAliceFree(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_TableComplaintBobNew(handle_t c_bob_data,
                                       uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<Range> demands(c_demand_count);
  for (uint64_t i = 0; i < c_demand_count; ++i) {
    demands[i].start = c_demand[i].start;
    demands[i].count = c_demand[i].count;
  }

  try {
    auto p = new Bob<BobData>(bob_data, self_id, peer_id, std::move(demands));
    CapiObject<Bob<BobData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableComplaintBobGetRequest(handle_t c_bob,
                                          char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Request request;
    bob->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableComplaintBobOnResponse(handle_t c_bob,
                                          char const* response_file,
                                          char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!bob->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableComplaintBobOnSecret(handle_t c_bob,
                                        char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return bob->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableComplaintBobGenerateClaim(handle_t c_bob,
                                             char const* claim_file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Claim claim;
    if (!bob->GenerateClaim(claim)) return false;

    yas::file_ostream os(claim_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(claim);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableComplaintBobSaveDecrypted(handle_t c_bob, char const* file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableComplaintBobFree(handle_t c_bob) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" complaint

// atomic_swap
extern "C" {
EXPORT handle_t E_TableAtomicSwapAliceNew(handle_t c_alice_data,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Alice<AliceData>(alice_data, self_id, peer_id);
    CapiObject<Alice<AliceData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableAtomicSwapAliceOnRequest(handle_t c_alice,
                                            char const* request_file,
                                            char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(request);

    Response response;
    if (!alice->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapAliceOnReceipt(handle_t c_alice,
                                            char const* receipt_file,
                                            char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!alice->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapAliceSetEvil(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_TableAtomicSwapAliceFree(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_TableAtomicSwapBobNew(handle_t c_bob_data,
                                        uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id,
                                        range_t const* c_demand,
                                        uint64_t c_demand_count) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<Range> demands(c_demand_count);
  for (uint64_t i = 0; i < c_demand_count; ++i) {
    demands[i].start = c_demand[i].start;
    demands[i].count = c_demand[i].count;
  }

  try {
    auto p = new Bob<BobData>(bob_data, self_id, peer_id, std::move(demands));
    CapiObject<Bob<BobData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableAtomicSwapBobGetRequest(handle_t c_bob,
                                           char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Request request;
    bob->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapBobOnResponse(handle_t c_bob,
                                           char const* response_file,
                                           char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!bob->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapBobOnSecret(handle_t c_bob,
                                         char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return bob->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapBobSaveDecrypted(handle_t c_bob,
                                              char const* file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableAtomicSwapBobFree(handle_t c_bob) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" atomic_swap

// ot_complaint
extern "C" {
EXPORT handle_t E_TableOtComplaintAliceNew(handle_t c_alice_data,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Alice<AliceData>(alice_data, self_id, peer_id);
    CapiObject<Alice<AliceData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtComplaintAliceGetNegoRequest(handle_t c_alice,
                                                  char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    NegoARequest request;
    alice->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintAliceOnNegoRequest(handle_t c_alice,
                                                 char const* request_file,
                                                 char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    NegoBRequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoBResponse response;
    if (!alice->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintAliceOnNegoResponse(handle_t c_alice,
                                                  char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    NegoAResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    if (!alice->OnNegoResponse(response)) return false;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintAliceOnRequest(handle_t c_alice,
                                             char const* request_file,
                                             char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(request);

    Response response;
    if (!alice->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintAliceOnReceipt(handle_t c_alice,
                                             char const* receipt_file,
                                             char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!alice->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintAliceSetEvil(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_TableOtComplaintAliceFree(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_TableOtComplaintBobNew(
    handle_t c_bob_data, uint8_t const* c_self_id, uint8_t const* c_peer_id,
    range_t const* c_demand, uint64_t c_demand_count, range_t const* c_phantom,
    uint64_t c_phantom_count) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<Range> demands(c_demand_count);
  for (uint64_t i = 0; i < c_demand_count; ++i) {
    demands[i].start = c_demand[i].start;
    demands[i].count = c_demand[i].count;
  }

  std::vector<Range> phantoms(c_phantom_count);
  for (uint64_t i = 0; i < c_phantom_count; ++i) {
    phantoms[i].start = c_phantom[i].start;
    phantoms[i].count = c_phantom[i].count;
  }

  try {
    auto p = new Bob<BobData>(bob_data, self_id, peer_id, std::move(demands),
                              std::move(phantoms));
    CapiObject<Bob<BobData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtComplaintBobGetNegoRequest(handle_t c_bob,
                                                char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    NegoBRequest request;
    bob->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintBobOnNegoRequest(handle_t c_bob,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    NegoARequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoAResponse response;
    if (!bob->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintBobOnNegoResponse(handle_t c_bob,
                                                char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    NegoBResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);
    return bob->OnNegoResponse(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintBobGetRequest(handle_t c_bob,
                                            char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Request request;
    bob->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintBobOnResponse(handle_t c_bob,
                                            char const* response_file,
                                            char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!bob->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintBobOnSecret(handle_t c_bob,
                                          char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return bob->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintBobGenerateClaim(handle_t c_bob,
                                               char const* claim_file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Claim claim;
    if (!bob->GenerateClaim(claim)) return false;

    yas::file_ostream os(claim_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(claim);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtComplaintBobSaveDecrypted(handle_t c_bob,
                                               char const* file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableOtComplaintBobFree(handle_t c_bob) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" ot_complaint

// ot_vrfq
extern "C" {
EXPORT handle_t E_TableOtVrfqAliceNew(handle_t c_alice_data,
                                      uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Alice(alice_data, self_id, peer_id);
    CapiObject<Alice>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtVrfqAliceGetNegoRequest(handle_t c_alice,
                                             char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  AlicePtr alice = CapiObject<Alice>::Get(c_alice);
  if (!alice) return false;

  try {
    NegoARequest request;
    alice->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqAliceOnNegoRequest(handle_t c_alice,
                                            char const* request_file,
                                            char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  AlicePtr alice = CapiObject<Alice>::Get(c_alice);
  if (!alice) return false;

  try {
    NegoBRequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoBResponse response;
    if (!alice->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqAliceOnNegoResponse(handle_t c_alice,
                                             char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  AlicePtr alice = CapiObject<Alice>::Get(c_alice);
  if (!alice) return false;

  try {
    NegoAResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    if (!alice->OnNegoResponse(response)) return false;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqAliceOnRequest(handle_t c_alice,
                                        char const* request_file,
                                        char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  AlicePtr alice = CapiObject<Alice>::Get(c_alice);
  if (!alice) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(request);

    Response response;
    if (!alice->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqAliceOnReceipt(handle_t c_alice,
                                        char const* receipt_file,
                                        char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  AlicePtr alice = CapiObject<Alice>::Get(c_alice);
  if (!alice) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!alice->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqAliceFree(handle_t c_alice) {
  using namespace scheme::table::ot_vrfq;
  return CapiObject<Alice>::Del(c_alice);
}

EXPORT handle_t
E_TableOtVrfqBobNew(handle_t c_bob_data, uint8_t const* c_self_id,
                    uint8_t const* c_peer_id, char const* c_query_key,
                    char const* c_query_values[], uint64_t c_query_value_count,
                    char const* c_phantoms[], uint64_t c_phantom_count) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<std::string> query_values(c_query_value_count);
  for (uint64_t i = 0; i < c_query_value_count; ++i) {
    query_values[i] = c_query_values[i];
  }

  std::vector<std::string> phantoms(c_phantom_count);
  for (uint64_t i = 0; i < c_phantom_count; ++i) {
    phantoms[i] = c_phantoms[i];
  }

  try {
    auto p = new Bob(bob_data, self_id, peer_id, c_query_key, query_values,
                     phantoms);
    CapiObject<Bob>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtVrfqBobGetNegoRequest(handle_t c_bob,
                                           char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    NegoBRequest request;
    bob->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqBobOnNegoRequest(handle_t c_bob,
                                          char const* request_file,
                                          char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    NegoARequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoAResponse response;
    if (!bob->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqBobOnNegoResponse(handle_t c_bob,
                                           char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    NegoBResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);
    return bob->OnNegoResponse(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqBobGetRequest(handle_t c_bob,
                                       char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    Request request;
    bob->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqBobOnResponse(handle_t c_bob,
                                       char const* response_file,
                                       char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!bob->OnResponse(response, receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqBobOnSecret(handle_t c_bob, char const* secret_file,
                                     char const* positions_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);

    std::vector<std::vector<uint64_t>> positions;
    if (!bob->OnSecret(secret, positions)) {
      assert(false);
      return false;
    }

    yas::file_ostream os(positions_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(positions);
    return true;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqBobFree(handle_t c_bob) {
  using namespace scheme::table::ot_vrfq;
  return CapiObject<Bob>::Del(c_bob);
}
}  // extern "C" ot_vrfq

// vrfq
extern "C" {
EXPORT handle_t E_TableVrfqAliceNew(handle_t c_alice_data,
                                    uint8_t const* c_self_id,
                                    uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Alice(alice_data, self_id, peer_id);
    CapiObject<Alice>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableVrfqAliceOnRequest(handle_t c_alice,
                                      char const* request_file,
                                      char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  AlicePtr alice = CapiObject<Alice>::Get(c_alice);
  if (!alice) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(request);

    Response response;
    if (!alice->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableVrfqAliceOnReceipt(handle_t c_alice,
                                      char const* receipt_file,
                                      char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  AlicePtr alice = CapiObject<Alice>::Get(c_alice);
  if (!alice) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!alice->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableVrfqAliceFree(handle_t c_alice) {
  using namespace scheme::table::vrfq;
  return CapiObject<Alice>::Del(c_alice);
}

EXPORT handle_t E_TableVrfqBobNew(handle_t c_bob_data, uint8_t const* c_self_id,
                                  uint8_t const* c_peer_id,
                                  char const* c_query_key,
                                  char const* c_query_values[],
                                  uint64_t c_query_value_count) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<std::string> query_values(c_query_value_count);
  for (uint64_t i = 0; i < c_query_value_count; ++i) {
    query_values[i] = c_query_values[i];
  }

  try {
    auto p = new Bob(bob_data, self_id, peer_id, c_query_key, query_values);
    CapiObject<Bob>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableVrfqBobGetRequest(handle_t c_bob, char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    Request request;
    bob->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableVrfqBobOnResponse(handle_t c_bob, char const* response_file,
                                     char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!bob->OnResponse(response, receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableVrfqBobOnSecret(handle_t c_bob, char const* secret_file,
                                   char const* positions_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  BobPtr bob = CapiObject<Bob>::Get(c_bob);
  if (!bob) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);

    std::vector<std::vector<uint64_t>> positions;
    if (!bob->OnSecret(secret, positions)) {
      assert(false);
      return false;
    }

    yas::file_ostream os(positions_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(positions);
    return true;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableVrfqBobFree(handle_t c_bob) {
  using namespace scheme::table::vrfq;
  return CapiObject<Bob>::Del(c_bob);
}
}  // extern "C" vrfq

// atomic_swap_vc
extern "C" {
EXPORT handle_t E_TableAtomicSwapVcAliceNew(handle_t c_alice_data,
                                            uint8_t const* c_self_id,
                                            uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Alice<AliceData>(alice_data, self_id, peer_id);
    CapiObject<Alice<AliceData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableAtomicSwapVcAliceOnRequest(handle_t c_alice,
                                              char const* request_file,
                                              char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(request);

    Response response;
    if (!alice->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapVcAliceOnReceipt(handle_t c_alice,
                                              char const* receipt_file,
                                              char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!alice->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapVcAliceSetEvil(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_TableAtomicSwapVcAliceFree(handle_t c_alice) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_TableAtomicSwapVcBobNew(handle_t c_bob_data,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id,
                                          range_t const* c_demand,
                                          uint64_t c_demand_count) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<Range> demands(c_demand_count);
  for (uint64_t i = 0; i < c_demand_count; ++i) {
    demands[i].start = c_demand[i].start;
    demands[i].count = c_demand[i].count;
  }

  try {
    auto p = new Bob<BobData>(bob_data, self_id, peer_id, std::move(demands));
    CapiObject<Bob<BobData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableAtomicSwapVcBobGetRequest(handle_t c_bob,
                                             char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Request request;
    bob->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapVcBobOnResponse(handle_t c_bob,
                                             char const* response_file,
                                             char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!bob->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapVcBobOnSecret(handle_t c_bob,
                                           char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return bob->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableAtomicSwapVcBobSaveDecrypted(handle_t c_bob,
                                                char const* file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableAtomicSwapVcBobFree(handle_t c_bob) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap_vc;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" atomic_swap_vc