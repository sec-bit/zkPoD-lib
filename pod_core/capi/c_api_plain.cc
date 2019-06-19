#include "c_api.h"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../scheme_atomic_swap_client.h"
#include "../scheme_atomic_swap_serialize.h"
#include "../scheme_atomic_swap_session.h"
#include "../scheme_complaint_client.h"
#include "../scheme_complaint_serialize.h"
#include "../scheme_complaint_session.h"
#include "../scheme_ot_complaint_client.h"
#include "../scheme_ot_complaint_serialize.h"
#include "../scheme_ot_complaint_session.h"
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

EXPORT bool E_PlainAliceBulletin(handle_t h, plain_bulletin_t* bulletin) {
  using namespace scheme::plain;
  AliceDataPtr a = CapiObject<AliceData>::Get(h);
  if (!a) return false;
  Bulletin const& v = a->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_PlainBobBulletin(handle_t h, plain_bulletin_t* bulletin) {
  using namespace scheme::plain;
  BobDataPtr b = CapiObject<BobData>::Get(h);
  if (!b) return false;
  Bulletin const& v = b->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  bulletin->size = v.size;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  return true;
}

EXPORT bool E_PlainAliceDataFree(handle_t h) {
  using namespace scheme::plain;
  return CapiObject<AliceData>::Del(h);
}

EXPORT bool E_PlainBobDataFree(handle_t h) {
  using namespace scheme::plain;
  return CapiObject<BobData>::Del(h);
}
}  // extern "C"

// complaint
extern "C" {
EXPORT handle_t E_PlainComplaintSessionNew(handle_t c_a,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  AliceDataPtr a = CapiObject<AliceData>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<AliceData>(a, self_id, peer_id);
    CapiObject<Session<AliceData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainComplaintSessionOnRequest(handle_t c_session,
                                             char const* request_file,
                                             char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainComplaintSessionOnReceipt(handle_t c_session,
                                             char const* receipt_file,
                                             char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainComplaintSessionSetEvil(handle_t c_session) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_PlainComplaintSessionFree(handle_t h) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  return CapiObject<Session<AliceData>>::Del(h);
}

EXPORT handle_t E_PlainComplaintClientNew(handle_t c_b,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id,
                                          range_t const* c_demand,
                                          uint64_t c_demand_count) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  BobDataPtr b = CapiObject<BobData>::Get(c_b);
  if (!b) return nullptr;

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
    auto p = new Client<BobData>(b, self_id, peer_id, std::move(demands));
    CapiObject<Client<BobData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainComplaintClientGetRequest(handle_t c_client,
                                             char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainComplaintClientOnResponse(handle_t c_client,
                                             char const* response_file,
                                             char const* receipt_file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainComplaintClientOnSecret(handle_t c_client,
                                           char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainComplaintClientGenerateClaim(handle_t c_client,
                                                char const* claim_file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Claim claim;
    if (!client->GenerateClaim(claim)) return false;

    yas::file_ostream os(claim_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(claim);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainComplaintClientSaveDecrypted(handle_t c_client,
                                                char const* file) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainComplaintClientFree(handle_t h) {
  using namespace scheme::plain;
  using namespace scheme::complaint;
  return CapiObject<Client<BobData>>::Del(h);
}
}  // extern "C" complaint

// atomic_swap
extern "C" {
EXPORT handle_t E_PlainAtomicSwapSessionNew(handle_t c_a,
                                            uint8_t const* c_self_id,
                                            uint8_t const* c_peer_id) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  AliceDataPtr a = CapiObject<AliceData>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<AliceData>(a, self_id, peer_id);
    CapiObject<Session<AliceData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainAtomicSwapSessionOnRequest(handle_t c_session,
                                              char const* request_file,
                                              char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainAtomicSwapSessionOnReceipt(handle_t c_session,
                                              char const* receipt_file,
                                              char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainAtomicSwapSessionSetEvil(handle_t c_session) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_PlainAtomicSwapSessionFree(handle_t h) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  return CapiObject<Session<AliceData>>::Del(h);
}

EXPORT handle_t E_PlainAtomicSwapClientNew(handle_t c_b,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id,
                                           range_t const* c_demand,
                                           uint64_t c_demand_count) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  BobDataPtr b = CapiObject<BobData>::Get(c_b);
  if (!b) return nullptr;

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
    auto p = new Client<BobData>(b, self_id, peer_id, std::move(demands));
    CapiObject<Client<BobData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainAtomicSwapClientGetRequest(handle_t c_client,
                                              char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainAtomicSwapClientOnResponse(handle_t c_client,
                                              char const* response_file,
                                              char const* receipt_file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainAtomicSwapClientOnSecret(handle_t c_client,
                                            char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainAtomicSwapClientSaveDecrypted(handle_t c_client,
                                                 char const* file) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainAtomicSwapClientFree(handle_t h) {
  using namespace scheme::plain;
  using namespace scheme::atomic_swap;
  return CapiObject<Client<BobData>>::Del(h);
}
}  // extern "C" atomic_swap

// ot_complaint
extern "C" {
EXPORT handle_t E_PlainOtComplaintSessionNew(handle_t c_a,
                                             uint8_t const* c_self_id,
                                             uint8_t const* c_peer_id) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  AliceDataPtr a = CapiObject<AliceData>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<AliceData>(a, self_id, peer_id);
    CapiObject<Session<AliceData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainOtComplaintSessionGetNegoRequest(handle_t c_session,
                                                    char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    NegoARequest request;
    session->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintSessionOnNegoRequest(handle_t c_session,
                                                   char const* request_file,
                                                   char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    NegoBRequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoBResponse response;
    if (!session->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintSessionOnNegoResponse(handle_t c_session,
                                                    char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    NegoAResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    if (!session->OnNegoResponse(response)) return false;
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintSessionOnRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    Request request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    Response response;
    if (!session->OnRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintSessionOnReceipt(handle_t c_session,
                                               char const* receipt_file,
                                               char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;

  try {
    Receipt receipt;
    yas::file_istream is(receipt_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(receipt);

    Secret secret;
    if (!session->OnReceipt(receipt, secret)) return false;

    yas::file_ostream os(secret_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintSessionSetEvil(handle_t c_session) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_PlainOtComplaintSessionFree(handle_t h) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  return CapiObject<Session<AliceData>>::Del(h);
}

EXPORT handle_t E_PlainOtComplaintClientNew(
    handle_t c_b, uint8_t const* c_self_id, uint8_t const* c_peer_id,
    range_t const* c_demand, uint64_t c_demand_count, range_t const* c_phantom,
    uint64_t c_phantom_count) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  BobDataPtr b = CapiObject<BobData>::Get(c_b);
  if (!b) return nullptr;

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
    auto p = new Client<BobData>(b, self_id, peer_id, std::move(demands),
                                 std::move(phantoms));
    CapiObject<Client<BobData>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_PlainOtComplaintClientGetNegoRequest(handle_t c_client,
                                                   char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    NegoBRequest request;
    client->GetNegoReqeust(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintClientOnNegoRequest(handle_t c_client,
                                                  char const* request_file,
                                                  char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    NegoARequest request;
    yas::file_istream is(request_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(request);

    NegoAResponse response;
    if (!client->OnNegoRequest(request, response)) return false;

    yas::file_ostream os(response_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintClientOnNegoResponse(handle_t c_client,
                                                   char const* response_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    NegoBResponse response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);
    return client->OnNegoResponse(response);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintClientGetRequest(handle_t c_client,
                                               char const* request_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Request request;
    client->GetRequest(request);
    yas::file_ostream os(request_file);
    yas::binary_oarchive<yas::file_ostream, YasBinF()> oa(os);
    oa.serialize(request);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintClientOnResponse(handle_t c_client,
                                               char const* response_file,
                                               char const* receipt_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(std::move(response), receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintClientOnSecret(handle_t c_client,
                                             char const* secret_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(secret);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintClientGenerateClaim(handle_t c_client,
                                                  char const* claim_file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    Claim claim;
    if (!client->GenerateClaim(claim)) return false;

    yas::file_ostream os(claim_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(claim);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_PlainOtComplaintClientSaveDecrypted(handle_t c_client,
                                                  char const* file) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_PlainOtComplaintClientFree(handle_t h) {
  using namespace scheme::plain;
  using namespace scheme::ot_complaint;
  return CapiObject<Client<BobData>>::Del(h);
}
}  // extern "C" ot_complaint
