#include "c_api.h"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../public/scheme_table.h"
#include "../scheme_ot_vrfq_client.h"
#include "../scheme_ot_vrfq_serialize.h"
#include "../scheme_ot_vrfq_session.h"
#include "../scheme_table_alice_data.h"
#include "../scheme_table_bob_data.h"
#include "../scheme_vrfq_client.h"
#include "../scheme_vrfq_serialize.h"
#include "../scheme_vrfq_session.h"

#include "../scheme_atomic_swap_client.h"
#include "../scheme_atomic_swap_serialize.h"
#include "../scheme_atomic_swap_session.h"

#include "../scheme_complaint_client.h"
#include "../scheme_complaint_serialize.h"
#include "../scheme_complaint_session.h"

#include "../scheme_ot_complaint_client.h"
#include "../scheme_ot_complaint_serialize.h"
#include "../scheme_ot_complaint_session.h"

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

EXPORT bool E_TableABulletin(handle_t h, table_bulletin_t* bulletin) {
  using namespace scheme::table;
  AliceDataPtr a = CapiObject<AliceData>::Get(h);
  if (!a) return false;
  Bulletin const& v = a->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  memcpy(bulletin->vrf_meta_digest, v.vrf_meta_digest.data(), 32);
  return true;
}

EXPORT bool E_TableBBulletin(handle_t h, table_bulletin_t* bulletin) {
  using namespace scheme::table;
  BobDataPtr b = CapiObject<BobData>::Get(h);
  if (!b) return false;
  Bulletin const& v = b->bulletin();
  bulletin->n = v.n;
  bulletin->s = v.s;
  memcpy(bulletin->sigma_mkl_root, v.sigma_mkl_root.data(), 32);
  memcpy(bulletin->vrf_meta_digest, v.vrf_meta_digest.data(), 32);
  return true;
}

EXPORT bool E_TableBIsKeyUnique(handle_t h, char const* query_key,
                                bool* unique) {
  using namespace scheme::table;
  BobDataPtr b = CapiObject<BobData>::Get(h);
  if (!b) return false;
  auto vrf_key = GetKeyMetaByName(b->vrf_meta(), query_key);
  if (!vrf_key) return false;
  *unique = vrf_key->unique;
  return true;
}

EXPORT bool E_TableAliceDataFree(handle_t h) {
  using namespace scheme::table;
  return CapiObject<AliceData>::Del(h);
}

EXPORT bool E_TableBobDataFree(handle_t h) {
  using namespace scheme::table;
  return CapiObject<BobData>::Del(h);
}

}  // extern "C"

// complaint
extern "C" {
EXPORT handle_t E_TableComplaintSessionNew(handle_t c_a,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintSessionOnRequest(handle_t c_session,
                                             char const* request_file,
                                             char const* response_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintSessionOnReceipt(handle_t c_session,
                                             char const* receipt_file,
                                             char const* secret_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintSessionSetEvil(handle_t c_session) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_TableComplaintSessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  return CapiObject<Session<AliceData>>::Del(h);
}

EXPORT handle_t E_TableComplaintClientNew(handle_t c_b,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id,
                                          range_t const* c_demand,
                                          uint64_t c_demand_count) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintClientGetRequest(handle_t c_client,
                                             char const* request_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintClientOnResponse(handle_t c_client,
                                             char const* response_file,
                                             char const* receipt_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintClientOnSecret(handle_t c_client,
                                           char const* secret_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintClientGenerateClaim(handle_t c_client,
                                                char const* claim_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableComplaintClientSaveDecrypted(handle_t c_client,
                                                char const* file) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableComplaintClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::complaint;
  return CapiObject<Client<BobData>>::Del(h);
}
}  // extern "C" complaint

// atomic_swap
extern "C" {
EXPORT handle_t E_TableAtomicSwapSessionNew(handle_t c_a,
                                            uint8_t const* c_self_id,
                                            uint8_t const* c_peer_id) {
  using namespace scheme::table;
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

EXPORT bool E_TableAtomicSwapSessionOnRequest(handle_t c_session,
                                              char const* request_file,
                                              char const* response_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableAtomicSwapSessionOnReceipt(handle_t c_session,
                                              char const* receipt_file,
                                              char const* secret_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableAtomicSwapSessionSetEvil(handle_t c_session) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_TableAtomicSwapSessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  return CapiObject<Session<AliceData>>::Del(h);
}

EXPORT handle_t E_TableAtomicSwapClientNew(handle_t c_b,
                                           uint8_t const* c_self_id,
                                           uint8_t const* c_peer_id,
                                           range_t const* c_demand,
                                           uint64_t c_demand_count) {
  using namespace scheme::table;
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

EXPORT bool E_TableAtomicSwapClientGetRequest(handle_t c_client,
                                              char const* request_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableAtomicSwapClientOnResponse(handle_t c_client,
                                              char const* response_file,
                                              char const* receipt_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableAtomicSwapClientOnSecret(handle_t c_client,
                                            char const* secret_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableAtomicSwapClientSaveDecrypted(handle_t c_client,
                                                 char const* file) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableAtomicSwapClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::atomic_swap;
  return CapiObject<Client<BobData>>::Del(h);
}
}  // extern "C" atomic_swap

// ot_complaint
extern "C" {
EXPORT handle_t E_TableOtComplaintSessionNew(handle_t c_a,
                                             uint8_t const* c_self_id,
                                             uint8_t const* c_peer_id) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintSessionGetNegoRequest(handle_t c_session,
                                                    char const* request_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintSessionOnNegoRequest(handle_t c_session,
                                                   char const* request_file,
                                                   char const* response_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintSessionOnNegoResponse(handle_t c_session,
                                                    char const* response_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintSessionOnRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintSessionOnReceipt(handle_t c_session,
                                               char const* receipt_file,
                                               char const* secret_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintSessionSetEvil(handle_t c_session) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto session = CapiObject<Session<AliceData>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_TableOtComplaintSessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  return CapiObject<Session<AliceData>>::Del(h);
}

EXPORT handle_t E_TableOtComplaintClientNew(
    handle_t c_b, uint8_t const* c_self_id, uint8_t const* c_peer_id,
    range_t const* c_demand, uint64_t c_demand_count, range_t const* c_phantom,
    uint64_t c_phantom_count) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientGetNegoRequest(handle_t c_client,
                                                   char const* request_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientOnNegoRequest(handle_t c_client,
                                                  char const* request_file,
                                                  char const* response_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientOnNegoResponse(handle_t c_client,
                                                   char const* response_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientGetRequest(handle_t c_client,
                                               char const* request_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientOnResponse(handle_t c_client,
                                               char const* response_file,
                                               char const* receipt_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientOnSecret(handle_t c_client,
                                             char const* secret_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientGenerateClaim(handle_t c_client,
                                                  char const* claim_file) {
  using namespace scheme::table;
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

EXPORT bool E_TableOtComplaintClientSaveDecrypted(handle_t c_client,
                                                  char const* file) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  auto client = CapiObject<Client<BobData>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableOtComplaintClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::ot_complaint;
  return CapiObject<Client<BobData>>::Del(h);
}
}  // extern "C" ot_complaint

// ot_vrfq
extern "C" {
EXPORT handle_t E_TableOtVrfqSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  AliceDataPtr a = CapiObject<AliceData>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session(a, self_id, peer_id);
    CapiObject<Session>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtVrfqSessionGetNegoRequest(handle_t c_session,
                                               char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  SessionPtr session = CapiObject<Session>::Get(c_session);
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

EXPORT bool E_TableOtVrfqSessionOnNegoRequest(handle_t c_session,
                                              char const* request_file,
                                              char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  SessionPtr session = CapiObject<Session>::Get(c_session);
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

EXPORT bool E_TableOtVrfqSessionOnNegoResponse(handle_t c_session,
                                               char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  SessionPtr session = CapiObject<Session>::Get(c_session);
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

EXPORT bool E_TableOtVrfqSessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  SessionPtr session = CapiObject<Session>::Get(c_session);
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

EXPORT bool E_TableOtVrfqSessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  SessionPtr session = CapiObject<Session>::Get(c_session);
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

EXPORT bool E_TableOtVrfqSessionFree(handle_t h) {
  using namespace scheme::table::ot_vrfq;
  return CapiObject<Session>::Del((Session*)h);
}

EXPORT handle_t E_TableOtVrfqClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       char const* c_query_key,
                                       char const* c_query_values[],
                                       uint64_t c_query_value_count,
                                       char const* c_phantoms[],
                                       uint64_t c_phantom_count) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  BobDataPtr b = CapiObject<BobData>::Get(c_b);
  if (!b) return nullptr;

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
    auto p =
        new Client(b, self_id, peer_id, c_query_key, query_values, phantoms);
    CapiObject<Client>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtVrfqClientGetNegoRequest(handle_t c_client,
                                              char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
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

EXPORT bool E_TableOtVrfqClientOnNegoRequest(handle_t c_client,
                                             char const* request_file,
                                             char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
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

EXPORT bool E_TableOtVrfqClientOnNegoResponse(handle_t c_client,
                                              char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
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

EXPORT bool E_TableOtVrfqClientGetRequest(handle_t c_client,
                                          char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
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

EXPORT bool E_TableOtVrfqClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(response, receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableOtVrfqClientOnSecret(handle_t c_client,
                                        char const* secret_file,
                                        char const* positions_file) {
  using namespace scheme::table;
  using namespace scheme::table::ot_vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);

    std::vector<std::vector<uint64_t>> positions;
    if (!client->OnSecret(secret, positions)) {
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

EXPORT bool E_TableOtVrfqClientFree(handle_t h) {
  using namespace scheme::table::ot_vrfq;
  return CapiObject<Client>::Del((Client*)h);
}
}  // extern "C" ot_vrfq

// vrfq
extern "C" {
EXPORT handle_t E_TableVrfqSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  AliceDataPtr a = CapiObject<AliceData>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session(a, self_id, peer_id);
    CapiObject<Session>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableVrfqSessionOnRequest(handle_t c_session,
                                        char const* request_file,
                                        char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  SessionPtr session = CapiObject<Session>::Get(c_session);
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

EXPORT bool E_TableVrfqSessionOnReceipt(handle_t c_session,
                                        char const* receipt_file,
                                        char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  SessionPtr session = CapiObject<Session>::Get(c_session);
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

EXPORT bool E_TableVrfqSessionFree(handle_t h) {
  using namespace scheme::table::vrfq;
  return CapiObject<Session>::Del((Session*)h);
}

EXPORT handle_t E_TableVrfqClientNew(handle_t c_b, uint8_t const* c_self_id,
                                     uint8_t const* c_peer_id,
                                     char const* c_query_key,
                                     char const* c_query_values[],
                                     uint64_t c_query_value_count) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  BobDataPtr b = CapiObject<BobData>::Get(c_b);
  if (!b) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  std::vector<std::string> query_values(c_query_value_count);
  for (uint64_t i = 0; i < c_query_value_count; ++i) {
    query_values[i] = c_query_values[i];
  }

  try {
    auto p = new Client(b, self_id, peer_id, c_query_key, query_values);
    CapiObject<Client>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableVrfqClientGetRequest(handle_t c_client,
                                        char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
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

EXPORT bool E_TableVrfqClientOnResponse(handle_t c_client,
                                        char const* response_file,
                                        char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
  if (!client) return false;

  try {
    Response response;
    yas::file_istream is(response_file);
    yas::binary_iarchive<yas::file_istream, YasBinF()> ia(is);
    ia.serialize(response);

    Receipt receipt;
    if (!client->OnResponse(response, receipt)) return false;

    yas::file_ostream os(receipt_file);
    yas::json_oarchive<yas::file_ostream> oa(os);
    oa.serialize(receipt);
  } catch (std::exception&) {
    return false;
  }

  return true;
}

EXPORT bool E_TableVrfqClientOnSecret(handle_t c_client,
                                      char const* secret_file,
                                      char const* positions_file) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  ClientPtr client = CapiObject<Client>::Get(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);

    std::vector<std::vector<uint64_t>> positions;
    if (!client->OnSecret(secret, positions)) {
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

EXPORT bool E_TableVrfqClientFree(handle_t h) {
  using namespace scheme::table::vrfq;
  return CapiObject<Client>::Del((Client*)h);
}
}  // extern "C" vrfq
