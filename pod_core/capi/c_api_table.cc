#include "c_api.h"

#include <algorithm>
#include <array>
#include <map>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../public/scheme_table.h"
#include "../scheme_table_a.h"
#include "../scheme_table_b.h"
#include "../scheme_table_otvrfq_client.h"
#include "../scheme_table_otvrfq_serialize.h"
#include "../scheme_table_otvrfq_session.h"
#include "../scheme_table_vrfq_client.h"
#include "../scheme_table_vrfq_serialize.h"
#include "../scheme_table_vrfq_session.h"

#include "../scheme_otbatch3_client.h"
#include "../scheme_otbatch3_serialize.h"
#include "../scheme_otbatch3_session.h"

#include "../scheme_batch3_client.h"
#include "../scheme_batch3_serialize.h"
#include "../scheme_batch3_session.h"

#include "../scheme_batch2_client.h"
#include "../scheme_batch2_serialize.h"
#include "../scheme_batch2_session.h"

#include "../scheme_batch_client.h"
#include "../scheme_batch_serialize.h"
#include "../scheme_batch_session.h"

#include "../scheme_otbatch_client.h"
#include "../scheme_otbatch_serialize.h"
#include "../scheme_otbatch_session.h"

#include "ecc.h"
#include "ecc_pub.h"

#include "c_api_object.h"

extern "C" {

EXPORT handle_t E_TableANew(char const* publish_path) {
  using namespace scheme::table;
  try {
    auto p = new A(publish_path);
    CapiObject<A>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT handle_t E_TableBNew(char const* bulletin_file,
                            char const* public_path) {
  using namespace scheme::table;
  try {
    auto p = new B(bulletin_file, public_path);
    CapiObject<B>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableABulletin(handle_t h, table_bulletin_t* bulletin) {
  using namespace scheme::table;
  APtr a = CapiObject<A>::Get(h);
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
  BPtr b = CapiObject<B>::Get(h);
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
  BPtr b = CapiObject<B>::Get(h);
  if (!b) return false;
  auto vrf_key = GetKeyMetaByName(b->vrf_meta(), query_key);
  if (!vrf_key) return false;
  *unique = vrf_key->unique;
  return true;
}

EXPORT bool E_TableAFree(handle_t h) {
  using namespace scheme::table;
  return CapiObject<A>::Del(h);
}

EXPORT bool E_TableBFree(handle_t h) {
  using namespace scheme::table;
  return CapiObject<B>::Del(h);
}

}  // extern "C"

// batch
extern "C" {
EXPORT handle_t E_TableBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::batch;
  APtr a = CapiObject<A>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<A>(a, self_id, peer_id);
    CapiObject<Session<A>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatchSessionOnRequest(handle_t c_session,
                                         char const* request_file,
                                         char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableBatchSessionOnReceipt(handle_t c_session,
                                         char const* receipt_file,
                                         char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableBatchSessionSetEvil(handle_t c_session) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto session = CapiObject<Session<A>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_TableBatchSessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::batch;
  return CapiObject<Session<A>>::Del(h);
}

EXPORT handle_t E_TableBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id,
                                      range_t const* c_demand,
                                      uint64_t c_demand_count) {
  using namespace scheme::table;
  using namespace scheme::batch;
  BPtr b = CapiObject<B>::Get(c_b);
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
    auto p = new Client<B>(b, self_id, peer_id, std::move(demands));
    CapiObject<Client<B>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatchClientGetRequest(handle_t c_client,
                                         char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatchClientOnResponse(handle_t c_client,
                                         char const* response_file,
                                         char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatchClientOnSecret(handle_t c_client,
                                       char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatchClientGenerateClaim(handle_t c_client,
                                            char const* claim_file) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatchClientSaveDecrypted(handle_t c_client,
                                            char const* file) {
  using namespace scheme::table;
  using namespace scheme::batch;
  auto client = CapiObject<Client<B>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableBatchClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::batch;
  return CapiObject<Client<B>>::Del(h);
}
}  // extern "C" batch

// batch2
extern "C" {
EXPORT handle_t E_TableBatch2SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  APtr a = CapiObject<A>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<A>(a, self_id, peer_id);
    CapiObject<Session<A>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatch2SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableBatch2SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableBatch2SessionSetEvil(handle_t c_session) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  auto session = CapiObject<Session<A>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_TableBatch2SessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  return CapiObject<Session<A>>::Del(h);
}

EXPORT handle_t E_TableBatch2ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  BPtr b = CapiObject<B>::Get(c_b);
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
    auto p = new Client<B>(b, self_id, peer_id, std::move(demands));
    CapiObject<Client<B>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatch2ClientGetRequest(handle_t c_client,
                                          char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatch2ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatch2ClientOnSecret(handle_t c_client,
                                        char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatch2ClientSaveDecrypted(handle_t c_client,
                                             char const* file) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  auto client = CapiObject<Client<B>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableBatch2ClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::batch2;
  return CapiObject<Client<B>>::Del(h);
}
}  // extern "C" batch2

// batch3
extern "C" {
EXPORT handle_t E_TableBatch3SessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  APtr a = CapiObject<A>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<A>(a, self_id, peer_id);
    CapiObject<Session<A>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatch3SessionOnRequest(handle_t c_session,
                                          char const* request_file,
                                          char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableBatch3SessionOnReceipt(handle_t c_session,
                                          char const* receipt_file,
                                          char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableBatch3SessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  return CapiObject<Session<A>>::Del(h);
}

EXPORT handle_t E_TableBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                       uint8_t const* c_peer_id,
                                       range_t const* c_demand,
                                       uint64_t c_demand_count) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  BPtr b = CapiObject<B>::Get(c_b);
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
    auto p = new Client<B>(b, self_id, peer_id, std::move(demands));
    CapiObject<Client<B>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableBatch3ClientGetRequest(handle_t c_client,
                                          char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatch3ClientOnResponse(handle_t c_client,
                                          char const* response_file,
                                          char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableBatch3ClientOnSecret(handle_t c_client,
                                        char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
  if (!client) return false;

  try {
    Secret secret;
    yas::file_istream is(secret_file);
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(secret);
    return client->OnSecret(std::move(secret));
  } catch (std::exception& e) {
    std::cerr << e.what() << "\n";
    return false;
  }

  return true;
}

EXPORT bool E_TableBatch3ClientSaveDecrypted(handle_t c_client,
                                             char const* file) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableBatch3ClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::batch3;
  return CapiObject<Client<B>>::Del(h);
}
}  // extern "C" batch3

// otbatch3
extern "C" {
EXPORT handle_t E_TableOtBatch3SessionNew(handle_t c_a,
                                          uint8_t const* c_self_id,
                                          uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  APtr a = CapiObject<A>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<A>(a, self_id, peer_id);
    CapiObject<Session<A>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtBatch3SessionGetNegoRequest(handle_t c_session,
                                                 char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatch3SessionOnNegoRequest(handle_t c_session,
                                                char const* request_file,
                                                char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatch3SessionOnNegoResponse(handle_t c_session,
                                                 char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatch3SessionOnRequest(handle_t c_session,
                                            char const* request_file,
                                            char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatch3SessionOnReceipt(handle_t c_session,
                                            char const* receipt_file,
                                            char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatch3SessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  return CapiObject<Session<A>>::Del(h);
}

EXPORT handle_t E_TableOtBatch3ClientNew(handle_t c_b, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id,
                                         range_t const* c_demand,
                                         uint64_t c_demand_count,
                                         range_t const* c_phantom,
                                         uint64_t c_phantom_count) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  BPtr b = CapiObject<B>::Get(c_b);
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
    auto p = new Client<B>(b, self_id, peer_id, std::move(demands),
                        std::move(phantoms));
    CapiObject<Client<B>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtBatch3ClientGetNegoRequest(handle_t c_client,
                                                char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatch3ClientOnNegoRequest(handle_t c_client,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatch3ClientOnNegoResponse(handle_t c_client,
                                                char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatch3ClientGetRequest(handle_t c_client,
                                            char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatch3ClientOnResponse(handle_t c_client,
                                            char const* response_file,
                                            char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatch3ClientOnSecret(handle_t c_client,
                                          char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatch3ClientSaveDecrypted(handle_t c_client,
                                               char const* file) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  auto client = CapiObject<Client<B>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableOtBatch3ClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::otbatch3;
  return CapiObject<Client<B>>::Del(h);
}
}  // extern "C" otbatch3

// otbatch
extern "C" {
EXPORT handle_t E_TableOtBatchSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                         uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  APtr a = CapiObject<A>::Get(c_a);
  if (!a) return nullptr;

  h256_t self_id;
  memcpy(self_id.data(), c_self_id, h256_t::size_value);
  h256_t peer_id;
  memcpy(peer_id.data(), c_peer_id, h256_t::size_value);

  try {
    auto p = new Session<A>(a, self_id, peer_id);
    CapiObject<Session<A>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtBatchSessionGetNegoRequest(handle_t c_session,
                                                char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatchSessionOnNegoRequest(handle_t c_session,
                                               char const* request_file,
                                               char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatchSessionOnNegoResponse(handle_t c_session,
                                                char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatchSessionOnRequest(handle_t c_session,
                                           char const* request_file,
                                           char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatchSessionOnReceipt(handle_t c_session,
                                           char const* receipt_file,
                                           char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto session = CapiObject<Session<A>>::Get(c_session);
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

EXPORT bool E_TableOtBatchSessionSetEvil(handle_t c_session) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto session = CapiObject<Session<A>>::Get(c_session);
  if (!session) return false;
  session->TestSetEvil();
  return true;
}

EXPORT bool E_TableOtBatchSessionFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  return CapiObject<Session<A>>::Del(h);
}

EXPORT handle_t E_TableOtBatchClientNew(handle_t c_b, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id,
                                        range_t const* c_demand,
                                        uint64_t c_demand_count,
                                        range_t const* c_phantom,
                                        uint64_t c_phantom_count) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  BPtr b = CapiObject<B>::Get(c_b);
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
    auto p = new Client<B>(b, self_id, peer_id, std::move(demands),
                        std::move(phantoms));
    CapiObject<Client<B>>::Add(p);
    return p;
  } catch (std::exception&) {
    return nullptr;
  }
}

EXPORT bool E_TableOtBatchClientGetNegoRequest(handle_t c_client,
                                               char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatchClientOnNegoRequest(handle_t c_client,
                                              char const* request_file,
                                              char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatchClientOnNegoResponse(handle_t c_client,
                                               char const* response_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatchClientGetRequest(handle_t c_client,
                                           char const* request_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatchClientOnResponse(handle_t c_client,
                                           char const* response_file,
                                           char const* receipt_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatchClientOnSecret(handle_t c_client,
                                         char const* secret_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatchClientGenerateClaim(handle_t c_client,
                                              char const* claim_file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
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

EXPORT bool E_TableOtBatchClientSaveDecrypted(handle_t c_client,
                                              char const* file) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  auto client = CapiObject<Client<B>>::Get(c_client);
  if (!client) return false;

  try {
    return client->SaveDecrypted(file);
  } catch (std::exception&) {
    return false;
  }
}

EXPORT bool E_TableOtBatchClientFree(handle_t h) {
  using namespace scheme::table;
  using namespace scheme::otbatch;
  return CapiObject<Client<B>>::Del(h);
}
}  // extern "C" otbatch

// otvrfq
extern "C" {
EXPORT handle_t E_TableOtVrfqSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                        uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::table::otvrfq;
  APtr a = CapiObject<A>::Get(c_a);
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
  BPtr b = CapiObject<B>::Get(c_b);
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
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
  using namespace scheme::table::otvrfq;
  return CapiObject<Client>::Del((Client*)h);
}
}  // extern "C" otvrfq

// vrfq
extern "C" {
EXPORT handle_t E_TableVrfqSessionNew(handle_t c_a, uint8_t const* c_self_id,
                                      uint8_t const* c_peer_id) {
  using namespace scheme::table;
  using namespace scheme::table::vrfq;
  APtr a = CapiObject<A>::Get(c_a);
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
  BPtr b = CapiObject<B>::Get(c_b);
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
