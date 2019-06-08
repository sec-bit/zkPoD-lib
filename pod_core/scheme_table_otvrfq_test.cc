#include "scheme_table_otvrfq_test.h"
#include "scheme_table.h"
#include "scheme_table_a.h"
#include "scheme_table_b.h"
#include "scheme_table_otvrfq_protocol.h"
#include "scheme_table_otvrfq_client.h"
#include "scheme_table_otvrfq_session.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just just two dummy values for test.
const h256_t kDummySessionId = h256_t{{1}};
const h256_t kDummyClientId = h256_t{{2}};
}  // namespace

namespace scheme::table::otvrfq {

bool QueryInternal(APtr a, BPtr b, std::string const& query_key,
                   std::vector<std::string> const& query_values,
                   std::vector<std::string> const& phantoms,
                   std::vector<std::vector<uint64_t>>& positions) {
  Session session(a, kDummySessionId, kDummyClientId);
  Client client(b, kDummyClientId, kDummySessionId, query_key, query_values,
                phantoms);

  NegoBRequest b_nego_request;
  client.GetNegoReqeust(b_nego_request);
  NegoBResponse b_nego_response;
  if (!session.OnNegoRequest(b_nego_request, b_nego_response)) {
    assert(false);
    return false;
  }
  if (!client.OnNegoResponse(b_nego_response)) {
    assert(false);
    return false;
  }

  NegoARequest a_nego_request;
  session.GetNegoReqeust(a_nego_request);
  NegoAResponse a_nego_response;
  if (!client.OnNegoRequest(a_nego_request, a_nego_response)) {
    assert(false);
    return false;
  }
  if (!session.OnNegoResponse(a_nego_response)) {
    assert(false);
    return false;
  }

  Request request;
  client.GetRequest(request);

  Response response;
  if (!session.OnRequest(request, response)) {
    assert(false);
    return false;
  }

  Receipt receipt;
  if (!client.OnResponse(response, receipt)) {
    assert(false);
    return false;
  }

  Secret secret;
  if (!session.OnReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!client.OnSecret(secret, positions)) {
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

bool Test(APtr a, BPtr b, std::string const& query_key,
          std::vector<std::string> const& query_values,
          std::vector<std::string> const& phantoms) {
  auto vrf_key = GetKeyMetaByName(b->vrf_meta(), query_key);
  if (!vrf_key) {
    std::cerr << "query_key: " << query_key << " not exist\n";
    return false;
  }
  bool unique = vrf_key->unique;

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
      if (!QueryInternal(a, b, query_key, values_with_suffix, phantoms_with_suffix,positions)) {
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
    if (!QueryInternal(a, b, query_key, query_values, phantoms, positions)) {
      assert(false);
      return false;
    }
    DumpPositions(query_key, query_values, positions);
  }
  return true;
}

bool Test(std::string const& publish_path, std::string const& /*output_path*/,
          std::string const& query_key,
          std::vector<std::string> const& query_values,
          std::vector<std::string> const& phantoms) {
  try {
    auto a = std::make_shared<A>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);

    return Test(a, b, query_key, query_values, phantoms);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::table::vrfq