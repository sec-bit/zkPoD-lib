#include "scheme_table_ot_vrfq_test_capi.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "../scheme_ot_vrfq_notary.h"
#include "../scheme_ot_vrfq_serialize.h"
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

class WrapperAlice {
 public:
  WrapperAlice(handle_t c_alice_data, uint8_t const* c_self_id,
               uint8_t const* c_peer_id) {
    h_ = E_TableOtVrfqAliceNew(c_alice_data, c_self_id, c_peer_id);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperAlice() {
    if (!E_TableOtVrfqAliceFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperBob {
 public:
  WrapperBob(handle_t c_bob_data, uint8_t const* c_self_id,
             uint8_t const* c_peer_id, char const* c_query_key,
             char const* c_query_values[], uint64_t c_query_value_count,
             char const* c_phantoms[], uint64_t c_phantom_count) {
    h_ = E_TableOtVrfqBobNew(c_bob_data, c_self_id, c_peer_id, c_query_key,
                             c_query_values, c_query_value_count, c_phantoms,
                             c_phantom_count);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperBob() {
    if (!E_TableOtVrfqBobFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};
}  // namespace

namespace scheme::table::ot_vrfq::capi {

bool QueryInternal(WrapperAliceData const& a, WrapperBobData const& b,
                   std::string const& output_path, std::string const& query_key,
                   std::vector<std::string> const& query_values,
                   std::vector<std::string> const& phantoms,
                   std::vector<std::vector<uint64_t>>& positions) {
  WrapperAlice alice(a.h(), kDummyAliceId.data(), kDummyBobId.data());
  std::vector<char const*> c_query_values(query_values.size());
  for (size_t i = 0; i < query_values.size(); ++i) {
    c_query_values[i] = query_values[i].c_str();
  }
  std::vector<char const*> c_phantoms(phantoms.size());
  for (size_t i = 0; i < phantoms.size(); ++i) {
    c_phantoms[i] = phantoms[i].c_str();
  }

  WrapperBob bob(b.h(), kDummyBobId.data(), kDummyAliceId.data(),
                 query_key.c_str(), c_query_values.data(),
                 c_query_values.size(), c_phantoms.data(), c_phantoms.size());

  std::string negoa_request_file = output_path + "/negoa_request";
  std::string negob_request_file = output_path + "/negob_request";
  std::string negoa_response_file = output_path + "/negoa_response";
  std::string negob_response_file = output_path + "/negob_response";
  std::string request_file = output_path + "/request";
  std::string response_file = output_path + "/response";
  std::string receipt_file = output_path + "/receipt";
  std::string secret_file = output_path + "/secret";
  std::string positions_file = output_path + "/positions";

  if (!E_TableOtVrfqBobGetNegoRequest(bob.h(), negob_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqAliceOnNegoRequest(alice.h(), negob_request_file.c_str(),
                                       negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqBobOnNegoResponse(bob.h(), negob_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqAliceGetNegoRequest(alice.h(),
                                        negoa_request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqBobOnNegoRequest(bob.h(), negoa_request_file.c_str(),
                                     negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqAliceOnNegoResponse(alice.h(),
                                        negoa_response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqBobGetRequest(bob.h(), request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqAliceOnRequest(alice.h(), request_file.c_str(),
                                   response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqBobOnResponse(bob.h(), response_file.c_str(),
                                  receipt_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqAliceOnReceipt(alice.h(), receipt_file.c_str(),
                                   secret_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableOtVrfqBobOnSecret(bob.h(), secret_file.c_str(),
                                positions_file.c_str())) {
    assert(false);
    return false;
  }

  try {
    yas::file_istream is(positions_file.c_str());
    yas::json_iarchive<yas::file_istream> ia(is);
    ia.serialize(positions);
  } catch (std::exception&) {
    assert(false);
    return false;
  }

  assert(positions.size() == query_values.size());

  return true;
}

void DumpPositions(std::string const& query_key,
                   std::vector<std::string> const& values,
                   std::vector<std::vector<uint64_t>> const& positions) {
  for (size_t i = 0; i < positions.size(); ++i) {
    auto const& position = positions[i];
    if (position.empty()) {
      std::cout << "Query " << query_key << " = " << values[i]
                << ": not exist\n";
    } else {
      std::cout << "Query " << query_key << " = " << values[i]
                << ", Positions: ";
      for (auto p : position) {
        std::cout << p << ";";
      }
      std::cout << "\n";
    }
  }
}

bool Test(WrapperAliceData const& a, WrapperBobData const& b,
          std::string const& output_path, std::string const& query_key,
          std::vector<std::string> const& query_values,
          std::vector<std::string> const& phantoms) {
  bool unique;

  if (!E_TableBIsKeyUnique(b.h(), query_key.c_str(), &unique)) return false;

  if (unique) {
    auto left_values = query_values;
    for (uint64_t i = 0;; ++i) {
      std::vector<std::string> values_with_suffix;
      for (auto const& v : left_values) {
        std::string value_with_suffix = v + "_" + std::to_string(i);
        values_with_suffix.emplace_back(std::move(value_with_suffix));
      }
      std::vector<std::string> phantoms_with_suffix;
      for (auto const& p : phantoms) {
        std::string phantom_with_suffix = p + "_" + std::to_string(i);
        phantoms_with_suffix.emplace_back(std::move(phantom_with_suffix));
      }
      std::vector<std::vector<uint64_t>> positions;
      if (!QueryInternal(a, b, output_path, query_key, values_with_suffix,
                         phantoms_with_suffix, positions)) {
        assert(false);
        return false;
      }
      DumpPositions(query_key, left_values, positions);

      for (uint64_t j = 0; j < positions.size(); ++j) {
        if (positions[j].empty()) left_values[j].clear();
      }

      left_values.erase(std::remove_if(left_values.begin(), left_values.end(),
                                       [](auto const& v) { return v.empty(); }),
                        left_values.end());
      if (left_values.empty()) break;
    }
  } else {
    std::vector<std::vector<uint64_t>> positions;
    if (!QueryInternal(a, b, output_path, query_key, query_values, phantoms,
                       positions)) {
      assert(false);
      return false;
    }
    DumpPositions(query_key, query_values, positions);
  }
  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          std::string const& query_key,
          std::vector<std::string> const& query_values,
          std::vector<std::string> const& phantoms) {
  try {
    WrapperAliceData alice_data(publish_path.c_str());
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    WrapperBobData bob_data(bulletin_file.c_str(), public_path.c_str());
    return Test(alice_data, bob_data, output_path, query_key, query_values,
                phantoms);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::table::ot_vrfq::capi