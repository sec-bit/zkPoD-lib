#include "scheme_table_ot_complaint_test_capi.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "../scheme_ot_complaint_notary.h"
#include "../scheme_ot_complaint_serialize.h"
#include "c_api.h"
#include "tick.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{{1}};
const h256_t kDummyClientId = h256_t{{2}};

class WrapperAliceData {
 public:
  WrapperAliceData(char const* publish_path) {
    h_ = E_TableAliceDataNew(publish_path);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperAliceData() {
    if (!E_TableAliceDataFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperBobData {
 public:
  WrapperBobData(char const* bulletin_file, char const* public_path) {
    h_ = E_TableBobDataNew(bulletin_file, public_path);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperBobData() {
    if (!E_TableBobDataFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperSession {
 public:
  WrapperSession(handle_t c_a, uint8_t const* c_self_id,
                 uint8_t const* c_peer_id) {
    h_ = E_TableOtComplaintSessionNew(c_a, c_self_id, c_peer_id);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperSession() {
    if (!E_TableOtComplaintSessionFree(h_)) abort();
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
    h_ =
        E_TableOtComplaintClientNew(c_b, c_self_id, c_peer_id, c_demand,
                                    c_demand_count, c_phantom, c_phantom_count);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperClient() {
    if (!E_TableOtComplaintClientFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};
}  // namespace

namespace scheme::table::ot_complaint::capi {

bool Test(std::string const& output_path, WrapperAliceData const& a,
          WrapperBobData const& b, std::vector<Range> const& demands,
          std::vector<Range> const& phantoms, bool evil) {
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

  if (evil) E_TableOtComplaintSessionSetEvil(session.h());

  std::string negoa_request_file = output_path + "/negoa_request";
  std::string negob_request_file = output_path + "/negob_request";
  std::string negoa_response_file = output_path + "/negoa_response";
  std::string negob_response_file = output_path + "/negob_response";
  std::string request_file = output_path + "/request";
  std::string response_file = output_path + "/response";
  std::string receipt_file = output_path + "/receipt";
  std::string secret_file = output_path + "/secret";
  std::string claim_file = output_path + "/claim";
  std::string output_file = output_path + "/decrypted_data";

  if (!E_TableOtComplaintClientGetNegoRequest(client.h(),
                                              negob_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintSessionOnNegoRequest(session.h(),
                                              negob_request_file.c_str(),
                                              negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintClientOnNegoResponse(client.h(),
                                              negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintSessionGetNegoRequest(session.h(),
                                               negoa_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintClientOnNegoRequest(client.h(),
                                             negoa_request_file.c_str(),
                                             negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintSessionOnNegoResponse(session.h(),
                                               negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintClientGetRequest(client.h(), request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintSessionOnRequest(session.h(), request_file.c_str(),
                                          response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintClientOnResponse(client.h(), response_file.c_str(),
                                          receipt_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtComplaintSessionOnReceipt(session.h(), receipt_file.c_str(),
                                          secret_file.c_str())) {
    assert(false);
    return false;
  }

  if (!evil) {
    if (!E_TableOtComplaintClientOnSecret(client.h(), secret_file.c_str())) {
      assert(false);
      return false;
    }

    if (!E_TableOtComplaintClientSaveDecrypted(client.h(),
                                               output_file.c_str())) {
      assert(false);
      return false;
    }
  } else {
    if (E_TableOtComplaintClientOnSecret(client.h(), secret_file.c_str())) {
      assert(false);
      return false;
    }

    if (!E_TableOtComplaintClientGenerateClaim(client.h(),
                                               claim_file.c_str())) {
      assert(false);
      return false;
    }

    using scheme::ot_complaint::Claim;
    using scheme::ot_complaint::Receipt;
    using scheme::ot_complaint::Secret;

    Claim claim;
    try {
      yas::file_istream is(claim_file.c_str());
      yas::json_iarchive<yas::file_istream> ia(is);
      ia.serialize(claim);
    } catch (std::exception&) {
      assert(false);
      return false;
    }

    Receipt receipt;
    try {
      yas::file_istream is(receipt_file.c_str());
      yas::json_iarchive<yas::file_istream> ia(is);
      ia.serialize(receipt);
    } catch (std::exception&) {
      assert(false);
      return false;
    }

    Secret secret;
    try {
      yas::file_istream is(secret_file.c_str());
      yas::json_iarchive<yas::file_istream> ia(is);
      ia.serialize(secret);
    } catch (std::exception&) {
      assert(false);
      return false;
    }

    std::cout << "claim: " << claim.i << "," << claim.j << "\n";
    table_bulletin_t c_bulletin;
    E_TableABulletin(a.h(), &c_bulletin);
    if (!VerifyClaim(c_bulletin.s, receipt, secret, claim)) {
      assert(false);
      return false;
    }
    std::cout << "verify claim success\n";
  }

  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, std::vector<Range> const& phantoms,
          bool test_evil) {
  try {
    WrapperAliceData a(publish_path.c_str());
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    WrapperBobData b(bulletin_file.c_str(), public_path.c_str());
    return Test(output_path, a, b, demands, phantoms, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::table::ot_complaint::capi