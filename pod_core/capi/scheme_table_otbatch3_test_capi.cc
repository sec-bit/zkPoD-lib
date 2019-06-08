#include "scheme_table_otbatch3_test_capi.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "c_api.h"
#include "tick.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{{1}};
const h256_t kDummyClientId = h256_t{{2}};

class WrapperA {
 public:
  WrapperA(char const* publish_path) {
    h_ = E_TableANew(publish_path);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperA() {
    if (!E_TableAFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperB {
 public:
  WrapperB(char const* bulletin_file, char const* public_path) {
    h_ = E_TableBNew(bulletin_file, public_path);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperB() {
    if (!E_TableBFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperSession {
 public:
  WrapperSession(handle_t c_a, uint8_t const* c_self_id,
                 uint8_t const* c_peer_id) {
    h_ = E_TableOtBatch3SessionNew(c_a, c_self_id, c_peer_id);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperSession() {
    if (!E_TableOtBatch3SessionFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperClient {
 public:
  WrapperClient(handle_t c_b, uint8_t const* c_self_id,
                uint8_t const* c_peer_id, range_t const* c_demand,
                uint64_t c_demand_count, range_t const* c_phantom,
                uint64_t c_phantom_count) {
    h_ = E_TableOtBatch3ClientNew(c_b, c_self_id, c_peer_id, c_demand,
                                c_demand_count, c_phantom, c_phantom_count);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperClient() {
    if (!E_TableOtBatch3ClientFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};
}  // namespace

namespace scheme::table::otbatch3::capi {

bool Test(std::string const& output_path, WrapperA const& a, WrapperB const& b,
          std::vector<Range> const& demands, std::vector<Range> const& phantoms) {
  Tick _tick_(__FUNCTION__);

  WrapperSession session(a.h(), kDummySessionId.data(), kDummyClientId.data());
  std::vector<range_t> c_demand(demands.size());
  for (size_t i = 0; i < demands.size(); ++i) {
    c_demand[i].start = demands[i].start;
    c_demand[i].count = demands[i].count;
  }
  std::vector<range_t> c_phantom(phantoms.size());
  for (size_t i = 0; i < phantoms.size(); ++i) {
    c_phantom[i].start = phantoms[i].start;
    c_phantom[i].count = phantoms[i].count;
  }

  WrapperClient client(b.h(), kDummyClientId.data(), kDummySessionId.data(),
                       c_demand.data(), c_demand.size(), c_phantom.data(),
                       c_phantom.size());

  std::string negoa_request_file = output_path + "/negoa_request";
  std::string negob_request_file = output_path + "/negob_request";
  std::string negoa_response_file = output_path + "/negoa_response";
  std::string negob_response_file = output_path + "/negob_response";
  std::string request_file = output_path + "/request";
  std::string response_file = output_path + "/response";
  std::string receipt_file = output_path + "/receipt";
  std::string secret_file = output_path + "/secret";
  std::string output_file = output_path + "/decrypted_data";

  if (!E_TableOtBatch3ClientGetNegoRequest(client.h(),
                                          negob_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3SessionOnNegoRequest(session.h(),
                                          negob_request_file.c_str(),
                                          negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3ClientOnNegoResponse(client.h(),
                                          negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3SessionGetNegoRequest(session.h(),
                                           negoa_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3ClientOnNegoRequest(client.h(), negoa_request_file.c_str(),
                                         negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3SessionOnNegoResponse(session.h(),
                                           negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3ClientGetRequest(client.h(), request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3SessionOnRequest(session.h(), request_file.c_str(),
                                     response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3ClientOnResponse(client.h(), response_file.c_str(),
                                     receipt_file.c_str())) {
    assert(false);
    return false;
  }


  if (!E_TableOtBatch3SessionOnReceipt(session.h(), receipt_file.c_str(),
                                     secret_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3ClientOnSecret(client.h(), secret_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtBatch3ClientSaveDecrypted(client.h(), output_file.c_str())) {
    assert(false);
    return false;
  }

  std::cout << "success: save to " << output_file << "\n";
  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands,std::vector<Range> const& phantoms) {
  try {
    WrapperA a(publish_path.c_str());
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    WrapperB b(bulletin_file.c_str(), public_path.c_str());
    return Test(output_path, a, b, demands, phantoms);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::table::otbatch3::capi
