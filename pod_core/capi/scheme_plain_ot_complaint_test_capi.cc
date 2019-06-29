#include "scheme_plain_ot_complaint_test_capi.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "../scheme_ot_complaint_notary.h"
#include "../scheme_ot_complaint_serialize.h"
#include "c_api.h"
#include "tick.h"

namespace {
// The alice id must be hash(AliceAddr), and the bob id must be hash(BobAddr).
// Here just just two dummy values for test.
const h256_t kDummyAliceId = h256_t{{1}};
const h256_t kDummyBobId = h256_t{{2}};

class WrapperAliceData {
 public:
  WrapperAliceData(char const* publish_path) {
    h_ = E_PlainAliceDataNew(publish_path);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperAliceData() {
    if (!E_PlainAliceDataFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperBobData {
 public:
  WrapperBobData(char const* bulletin_file, char const* public_path) {
    h_ = E_PlainBobDataNew(bulletin_file, public_path);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperBobData() {
    if (!E_PlainBobDataFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperAlice {
 public:
  WrapperAlice(handle_t c_alice_data, uint8_t const* c_self_id,
               uint8_t const* c_peer_id) {
    h_ = E_PlainOtComplaintAliceNew(c_alice_data, c_self_id, c_peer_id);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperAlice() {
    if (!E_PlainOtComplaintAliceFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperBob {
 public:
  WrapperBob(handle_t c_bob_data, uint8_t const* c_self_id,
             uint8_t const* c_peer_id, range_t const* c_demand,
             uint64_t c_demand_count, range_t const* c_phantom,
             uint64_t c_phantom_count) {
    h_ = E_PlainOtComplaintBobNew(c_bob_data, c_self_id, c_peer_id, c_demand,
                                  c_demand_count, c_phantom, c_phantom_count);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperBob() {
    if (!E_PlainOtComplaintBobFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};
}  // namespace

namespace scheme::plain::ot_complaint::capi {

bool Test(std::string const& output_path, WrapperAliceData const& a,
          WrapperBobData const& b, std::vector<Range> const& demands,
          std::vector<Range> const& phantoms, bool evil) {
  Tick _tick_(__FUNCTION__);

  WrapperAlice alice(a.h(), kDummyAliceId.data(), kDummyBobId.data());
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

  WrapperBob bob(b.h(), kDummyBobId.data(), kDummyAliceId.data(),
                 c_demand.data(), c_demand.size(), c_phantom.data(),
                 c_phantom.size());

  if (evil) E_PlainOtComplaintAliceSetEvil(alice.h());

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

  if (!E_PlainOtComplaintBobGetNegoRequest(bob.h(),
                                           negob_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintAliceOnNegoRequest(
          alice.h(), negob_request_file.c_str(), negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintBobOnNegoResponse(bob.h(),
                                           negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintAliceGetNegoRequest(alice.h(),
                                             negoa_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintBobOnNegoRequest(bob.h(), negoa_request_file.c_str(),
                                          negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintAliceOnNegoResponse(alice.h(),
                                             negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintBobGetRequest(bob.h(), request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintAliceOnRequest(alice.h(), request_file.c_str(),
                                        response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintBobOnResponse(bob.h(), response_file.c_str(),
                                       receipt_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainOtComplaintAliceOnReceipt(alice.h(), receipt_file.c_str(),
                                        secret_file.c_str())) {
    assert(false);
    return false;
  }

  if (!evil) {
    if (!E_PlainOtComplaintBobOnSecret(bob.h(), secret_file.c_str())) {
      assert(false);
      return false;
    }

    if (!E_PlainOtComplaintBobSaveDecrypted(bob.h(), output_file.c_str())) {
      assert(false);
      return false;
    }
  } else {
    if (E_PlainOtComplaintBobOnSecret(bob.h(), secret_file.c_str())) {
      assert(false);
      return false;
    }

    if (!E_PlainOtComplaintBobGenerateClaim(bob.h(), claim_file.c_str())) {
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
    plain_bulletin_t c_bulletin;
    E_PlainAliceBulletin(a.h(), &c_bulletin);
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
    WrapperAliceData alice_data(publish_path.c_str());
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    WrapperBobData bob_data(bulletin_file.c_str(), public_path.c_str());
    return Test(output_path, alice_data, bob_data, demands, phantoms,
                test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::plain::ot_complaint::capi