#include "scheme_atomic_swap_vc_test.h"
#include "scheme_atomic_swap_vc_alice.h"
#include "scheme_atomic_swap_vc_bob.h"
#include "scheme_atomic_swap_vc_notary.h"
#include "scheme_atomic_swap_vc_protocol.h"
#include "scheme_plain.h"
#include "scheme_plain_alice_data.h"
#include "scheme_plain_bob_data.h"
#include "scheme_table.h"
#include "scheme_table_alice_data.h"
#include "scheme_table_bob_data.h"

namespace {
// The alice id must be hash(AliceAddr), and the bob id must be hash(BobAddr).
// Here just use two dummy values for test.
const h256_t kDummyAliceId = h256_t{{1}};
const h256_t kDummyBobId = h256_t{{2}};
}  // namespace

namespace scheme::atomic_swap_vc {

template <typename AliceData, typename BobData>
bool Test(std::string const& output_path, std::shared_ptr<AliceData> alice_data,
          std::shared_ptr<BobData> bob_data, std::vector<Range> const& demands,
          bool evil) {
  Tick _tick_(__FUNCTION__);

  auto output_file = output_path + "/decrypted_data";

  Alice alice(alice_data, kDummyAliceId, kDummyBobId);
  Bob bob(bob_data, kDummyBobId, kDummyAliceId, demands);
  if (evil) alice.TestSetEvil();

  Request request;
  bob.GetRequest(request);

  Response response;
  if (!alice.OnRequest(request, response)) {
    assert(false);
    return false;
  }

  Receipt receipt;
  if (!bob.OnResponse(std::move(response), receipt)) {
    assert(false);
    return false;
  }

  Secret secret;
  if (!alice.OnReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!evil) {
    if (!bob.OnSecret(secret)) {
      assert(false);
      return false;
    }

    if (!bob.SaveDecrypted(output_file)) {
      assert(false);
      return false;
    }

    std::cout << "success: save to " << output_file << "\n";
  } else {
    if (bob.OnSecret(secret)) {
      assert(false);
      return false;
    }
  }

  return true;
}

}  // namespace scheme::atomic_swap_vc

namespace scheme::plain::atomic_swap_vc {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, bool test_evil) {
  try {
    using scheme::plain::AliceData;
    using scheme::plain::BobData;
    auto alice_data = std::make_shared<AliceData>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto bob_data = std::make_shared<BobData>(bulletin_file, public_path);
    auto const& bulletin = bob_data->bulletin();
    std::cout << "n: " << bulletin.n << ", s: " << bulletin.s
              << ", size: " << bulletin.size << "\n";
    return scheme::atomic_swap_vc::Test(output_path, alice_data, bob_data,
                                        demands, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme::plain::atomic_swap_vc

namespace scheme::table::atomic_swap_vc {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, bool test_evil) {
  try {
    using scheme::table::AliceData;
    using scheme::table::BobData;
    auto alice_data = std::make_shared<AliceData>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto bob_data = std::make_shared<BobData>(bulletin_file, public_path);
    auto const& bulletin = bob_data->bulletin();
    std::cout << "n: " << bulletin.n << ", s: " << bulletin.s << "\n";
    return scheme::atomic_swap_vc::Test(output_path, alice_data, bob_data,
                                        demands, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme::table::atomic_swap_vc