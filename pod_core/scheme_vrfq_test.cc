#include "scheme_vrfq_test.h"
#include "scheme_table.h"
#include "scheme_table_alice_data.h"
#include "scheme_table_bob_data.h"
#include "scheme_vrfq_alice.h"
#include "scheme_vrfq_bob.h"
#include "scheme_vrfq_protocol.h"

namespace {
// The alice id must be hash(AliceAddr), and the bob id must be hash(BobAddr).
// Here just just two dummy values for test.
const h256_t kDummyAliceId = h256_t{{1}};
const h256_t kDummyBobId = h256_t{{2}};
}  // namespace

namespace scheme::table::vrfq {

bool QueryInternal(AliceDataPtr alice_data, BobDataPtr bob_data,
                   std::string const& query_key,
                   std::vector<std::string> const& query_values,
                   std::vector<std::vector<uint64_t>>& positions) {
  Alice alice(alice_data, kDummyAliceId, kDummyBobId);
  Bob bob(bob_data, kDummyBobId, kDummyAliceId, query_key, query_values);

  Request request;
  bob.GetRequest(request);

  Response response;
  if (!alice.OnRequest(request, response)) {
    assert(false);
    return false;
  }

  Receipt receipt;
  if (!bob.OnResponse(response, receipt)) {
    assert(false);
    return false;
  }

  Secret secret;
  if (!alice.OnReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!bob.OnSecret(secret, positions)) {
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

bool Test(AliceDataPtr alice_data, BobDataPtr bob_data,
          std::string const& query_key,
          std::vector<std::string> const& query_values) {
  auto vrf_key = GetKeyMetaByName(bob_data->vrf_meta(), query_key);
  if (!vrf_key) {
    std::cerr << "query_key: " << query_key << " not exist\n";
    return false;
  }
  bool unique = vrf_key->unique;

  if (unique) {
    auto left_values = query_values;
    for (uint64_t i = 0;; ++i) {
      std::vector<std::string> values_with_suffix;
      for (auto const& value : left_values) {
        std::string value_with_suffix = value + "_" + std::to_string(i);
        values_with_suffix.emplace_back(std::move(value_with_suffix));
      }
      std::vector<std::vector<uint64_t>> positions;
      if (!QueryInternal(alice_data, bob_data, query_key, values_with_suffix,
                         positions))
        return false;
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
    if (!QueryInternal(alice_data, bob_data, query_key, query_values,
                       positions))
      return false;
    DumpPositions(query_key, query_values, positions);
  }
  return true;
}

bool Test(std::string const& publish_path, std::string const& /*output_path*/,
          std::string const& query_key,
          std::vector<std::string> const& query_values) {
  try {
    auto alice_data = std::make_shared<AliceData>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto bob_data = std::make_shared<BobData>(bulletin_file, public_path);

    return Test(alice_data, bob_data, query_key, query_values);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::table::vrfq