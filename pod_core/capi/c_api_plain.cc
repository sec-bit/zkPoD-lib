#include "c_api.h"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

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
#include "../scheme_plain_alice_data.h"
#include "../scheme_plain_bob_data.h"

#include "ecc.h"
#include "ecc_pub.h"

#include "c_api_object.h"

extern "C" {
EXPORT handle_t E_PlainAliceDataNew(char const* publish_path) {
  using namespace scheme::plain;
  try {
    auto p = new AliceData(publish_path);
    CapiObject<AliceData>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT handle_t E_PlainBobDataNew(char const* bulletin_file,
                                  char const* public_path) {
  using namespace scheme::plain;
  try {
    auto p = new BobData(bulletin_file, public_path);
    CapiObject<BobData>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainAliceBulletin(handle_t c_alice_data,
                                 plain_bulletin_t* bulletin) {
  using namespace scheme::plain;
  AliceDataPtr alice_data = CapiObject<AliceData>::Get(c_alice_data);
  if (!alice_data) return false;
  Bulletin const& v = alice_data->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_PlainBobBulletin(handle_t c_bob_data,
                               plain_bulletin_t* bulletin) {
  using namespace scheme::plain;
  BobDataPtr bob_data = CapiObject<BobData>::Get(c_bob_data);
  if (!bob_data) return false;
  Bulletin const& v = bob_data->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_PlainAliceDataFree(handle_t c_alice_data) {
  using namespace scheme::plain;
  return CapiObject<AliceData>::Del(c_alice_data);
}

EXPORT bool E_PlainBobDataFree(handle_t c_bob_data) {
  using namespace scheme::plain;
  return CapiObject<BobData>::Del(c_bob_data);
}
}  // extern "C"

// complaint
extern "C" {
EXPORT handle_t E_PlainComplaintAliceNew(handle_t c_alice_data,
                                         uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintAliceOnRequest(handle_t c_alice,
                                           char const* request_file,
                                           char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintAliceOnReceipt(handle_t c_alice,
                                           char const* receipt_file,
                                           char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintAliceSetEvil(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_PlainComplaintAliceFree(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_PlainComplaintBobNew(handle_t c_bob_data,
                                       uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintBobGetRequest(handle_t c_bob,
                                          char const* request_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintBobOnResponse(handle_t c_bob,
                                          char const* response_file,
                                          char const* receipt_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintBobOnSecret(handle_t c_bob,
                                        char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintBobGenerateClaim(handle_t c_bob,
                                             char const* claim_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainComplaintBobSaveDecrypted(handle_t c_bob, char const* file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainComplaintBobFree(handle_t c_bob) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" complaint

// atomic_swap
extern "C" {
EXPORT handle_t E_PlainAtomicSwapAliceNew(handle_t c_alice_data,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapAliceOnRequest(handle_t c_alice,
                                            char const* request_file,
                                            char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapAliceOnReceipt(handle_t c_alice,
                                            char const* receipt_file,
                                            char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapAliceSetEvil(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_PlainAtomicSwapAliceFree(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_PlainAtomicSwapBobNew(handle_t c_bob_data,
                                        uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id,
                                        range_t const* c_demand,
                                        uint64_t c_demand_count) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapBobGetRequest(handle_t c_bob,
                                           char const* request_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapBobOnResponse(handle_t c_bob,
                                           char const* response_file,
                                           char const* receipt_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapBobOnSecret(handle_t c_bob,
                                         char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapBobSaveDecrypted(handle_t c_bob,
                                              char const* file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainAtomicSwapBobFree(handle_t c_bob) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" atomic_swap

// ot_complaint
extern "C" {
EXPORT handle_t E_PlainOtComplaintAliceNew(handle_t c_alice_data,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintAliceGetNegoRequest(handle_t c_alice,
                                                  char const* request_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintAliceOnNegoRequest(handle_t c_alice,
                                                 char const* request_file,
                                                 char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintAliceOnNegoResponse(handle_t c_alice,
                                                  char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintAliceOnRequest(handle_t c_alice,
                                             char const* request_file,
                                             char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintAliceOnReceipt(handle_t c_alice,
                                             char const* receipt_file,
                                             char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintAliceSetEvil(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_PlainOtComplaintAliceFree(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_PlainOtComplaintBobNew(
    handle_t c_bob_data, uint8_t const* c_self_id, uint8_t const* c_peer_id,
    range_t const* c_demand, uint64_t c_demand_count, range_t const* c_phantom,
    uint64_t c_phantom_count) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobGetNegoRequest(handle_t c_bob,
                                                char const* request_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobOnNegoRequest(handle_t c_bob,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobOnNegoResponse(handle_t c_bob,
                                                char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobGetRequest(handle_t c_bob,
                                            char const* request_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobOnResponse(handle_t c_bob,
                                            char const* response_file,
                                            char const* receipt_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobOnSecret(handle_t c_bob,
                                          char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobGenerateClaim(handle_t c_bob,
                                               char const* claim_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainOtComplaintBobSaveDecrypted(handle_t c_bob,
                                               char const* file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainOtComplaintBobFree(handle_t c_bob) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" ot_complaint

// atomic_swap_vc
extern "C" {
EXPORT handle_t E_PlainAtomicSwapVcAliceNew(handle_t c_alice_data,
                                            uint8_t const* c_self_id,
                                            uint8_t const* c_peer_id) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapVcAliceOnRequest(handle_t c_alice,
                                              char const* request_file,
                                              char const* response_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapVcAliceOnReceipt(handle_t c_alice,
                                              char const* receipt_file,
                                              char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapVcAliceSetEvil(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap_vc;
  auto alice = CapiObject<Alice<AliceData>>::Get(c_alice);
  if (!alice) return false;
  alice->TestSetEvil();
  return true;
}

EXPORT bool E_PlainAtomicSwapVcAliceFree(handle_t c_alice) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap_vc;
  return CapiObject<Alice<AliceData>>::Del(c_alice);
}

EXPORT handle_t E_PlainAtomicSwapVcBobNew(handle_t c_bob_data,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id,
                                          range_t const* c_demand,
                                          uint64_t c_demand_count) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapVcBobGetRequest(handle_t c_bob,
                                             char const* request_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapVcBobOnResponse(handle_t c_bob,
                                             char const* response_file,
                                             char const* receipt_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapVcBobOnSecret(handle_t c_bob,
                                           char const* secret_file) {
  using namespace scheme::plain;
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

EXPORT bool E_PlainAtomicSwapVcBobSaveDecrypted(handle_t c_bob,
                                                char const* file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap_vc;
  auto bob = CapiObject<Bob<BobData>>::Get(c_bob);
  if (!bob) return false;

  try {
    return bob->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainAtomicSwapVcBobFree(handle_t c_bob) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap_vc;
  return CapiObject<Bob<BobData>>::Del(c_bob);
}
}  // extern "C" atomic_swap_vc