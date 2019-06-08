#include "scheme_table_vrfq_test_capi.h"

#include <cassert>
#include <iostream>
#include <memory>

#include "../scheme_table_vrfq_notary.h"
#include "../scheme_table_vrfq_serialize.h"
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
    h_ = E_TableVrfqSessionNew(c_a, c_self_id, c_peer_id);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperSession() {
    if (!E_TableVrfqSessionFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};

class WrapperClient {
 public:
  WrapperClient(handle_t c_b, uint8_t const* c_self_id,
                uint8_t const* c_peer_id, char const* c_query_key,
                char const* c_query_values[], uint64_t c_query_value_count) {
    h_ = E_TableVrfqClientNew(c_b, c_self_id, c_peer_id, c_query_key,
                                c_query_values, c_query_value_count);
    if (!h_) throw std::runtime_error("");
  }
  ~WrapperClient() {
    if (!E_TableVrfqClientFree(h_)) abort();
  }
  handle_t h() const { return h_; }

 private:
  handle_t h_;
};
}  // namespace

namespace scheme::table::vrfq::capi {

bool QueryInternal(WrapperA const& a, WrapperB const& b,
                   std::string const& output_path, std::string const& query_key,
                   std::vector<std::string> const& query_values,
                   std::vector<std::vector<uint64_t>>& positions) {
  WrapperSession session(a.h(), kDummySessionId.data(), kDummyClientId.data());
  std::vector<char const*> c_query_values(query_values.size());
  for (size_t i = 0; i < query_values.size(); ++i) {
    c_query_values[i] = query_values[i].c_str();
  }

  WrapperClient client(b.h(), kDummyClientId.data(), kDummySessionId.data(),
                       query_key.c_str(), c_query_values.data(),
                       c_query_values.size());

  std::string request_file = output_path + "/request";
  std::string response_file = output_path + "/response";
  std::string receipt_file = output_path + "/receipt";
  std::string secret_file = output_path + "/secret";
  std::string positions_file = output_path + "/positions";

  if (!E_TableVrfqClientGetRequest(client.h(), request_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableVrfqSessionOnRequest(session.h(), request_file.c_str(),
                                     response_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableVrfqClientOnResponse(client.h(), response_file.c_str(),
                                     receipt_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableVrfqSessionOnReceipt(session.h(), receipt_file.c_str(),
                                     secret_file.c_str())) {
    assert(false);
    return false;
  }

  if (!E_TableVrfqClientOnSecret(client.h(), secret_file.c_str(),
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

bool Test(WrapperA const& a, WrapperB const& b, std::string const& output_path,
          std::string const& query_key,
          std::vector<std::string> const& query_values) {
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
      std::vector<std::vector<uint64_t>> positions;
      if (!QueryInternal(a, b, output_path, query_key, values_with_suffix,
                         positions)) {
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
    if (!QueryInternal(a, b, output_path, query_key, query_values, positions)) {
      assert(false);
      return false;
    }
    DumpPositions(query_key, query_values, positions);
  }
  return true;
}

bool Test(std::string const& publish_path, std::string const& output_path,
          std::string const& query_key,
          std::vector<std::string> const& query_values) {
  try {
    WrapperA a(publish_path.c_str());
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    WrapperB b(bulletin_file.c_str(), public_path.c_str());
    return Test(a, b, output_path, query_key, query_values);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::table::vrfq::capi