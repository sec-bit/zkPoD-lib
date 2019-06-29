#include "scheme_plain_atomic_swap_vc_test_capi.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "../scheme_atomic_swap_vc_notary.h"
#include "../scheme_atomic_swap_vc_serialize.h"
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
    h_ = E_PlainAtomicSwapVcAliceNew(c_alice_data, c_self_id, c_peer_id);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperAlice() {
    if (!E_PlainAtomicSwapVcAliceFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperBob {
 public:
  WrapperBob(handle_t c_bob_data, uint8_t const* c_self_id,
             uint8_t const* c_peer_id, range_t const* c_demand,
             uint64_t c_demand_count) {
    h_ = E_PlainAtomicSwapVcBobNew(c_bob_data, c_self_id, c_peer_id, c_demand,
                                 c_demand_count);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperBob() {
    if (!E_PlainAtomicSwapVcBobFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};
}  // namespace

namespace scheme::plain::atomic_swap_vc::capi {

bool Test(std::string const& output_path, WrapperAliceData const& a,
          WrapperBobData const& b, std::vector<Range> const& demands,
          bool evil) {
  Tick _tick_(__FUNCTION__);

  WrapperAlice alice(a.h(), kDummyAliceId.data(), kDummyBobId.data());
  std::vector<range_t> c_demand(demands.size());
  for (size_t i = 0; i < demands.size(); ++i) {
    c_demand[i].start = demands[i].start;
    c_demand[i].count = demands[i].count;
  }
  WrapperBob bob(b.h(), kDummyBobId.data(), kDummyAliceId.data(),
                 c_demand.data(), c_demand.size());
  if (evil) E_PlainAtomicSwapVcAliceSetEvil(alice.h());

  std::string request_file = output_path + "/request";
  std::string response_file = output_path + "/response";
  std::string receipt_file = output_path + "/receipt";
  std::string secret_file = output_path + "/secret";
  std::string output_file = output_path + "/decrypted_data";

  if (!E_PlainAtomicSwapVcBobGetRequest(bob.h(), request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainAtomicSwapVcAliceOnRequest(alice.h(), request_file.c_str(),
                                       response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainAtomicSwapVcBobOnResponse(bob.h(), response_file.c_str(),
                                      receipt_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_PlainAtomicSwapVcAliceOnReceipt(alice.h(), receipt_file.c_str(),
                                       secret_file.c_str())) {
    assert(false);
    return false;
  }

  if (!evil) {
    if (!E_PlainAtomicSwapVcBobOnSecret(bob.h(), secret_file.c_str())) {
      assert(false);
      return false;
    }

    if (!E_PlainAtomicSwapVcBobSaveDecrypted(bob.h(), output_file.c_str())) {
      assert(false);
      return false;
    }
  } else {
    if (E_PlainAtomicSwapVcBobOnSecret(bob.h(), secret_file.c_str())) {
      assert(false);
      return false;
    }
  }

  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, bool test_evil) {
  try {
    WrapperAliceData alice_data(publish_path.c_str());
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    WrapperBobData bob_data(bulletin_file.c_str(), public_path.c_str());
    return Test(output_path, alice_data, bob_data, demands, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::plain::atomic_swap_vc::capi