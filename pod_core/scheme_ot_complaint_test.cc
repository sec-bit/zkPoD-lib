#include "scheme_ot_complaint_test.h"
#include "scheme_ot_complaint_alice.h"
#include "scheme_ot_complaint_bob.h"
#include "scheme_ot_complaint_notary.h"
#include "scheme_ot_complaint_protocol.h"
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

namespace scheme::ot_complaint {

template <typename AliceData, typename BobData>
bool Test(std::string const& output_path, std::shared_ptr<AliceData> alice_data,
          std::shared_ptr<BobData> bob_data, std::vector<Range> const& demands,
          std::vector<Range> const& phantoms, bool evil) {
  Tick _tick_(__FUNCTION__);

  auto output_file = output_path + "/decrypted_data";

  Alice alice(alice_data, kDummyAliceId, kDummyBobId);
  Bob bob(bob_data, kDummyBobId, kDummyAliceId, demands, phantoms);
  if (evil) alice.TestSetEvil();

  NegoBRequest b_nego_request;
  bob.GetNegoReqeust(b_nego_request);
  NegoBResponse b_nego_response;
  if (!alice.OnNegoRequest(b_nego_request, b_nego_response)) {
    assert(false);
    return false;
  }
  if (!bob.OnNegoResponse(b_nego_response)) {
    assert(false);
    return false;
  }

  NegoARequest a_nego_request;
  alice.GetNegoReqeust(a_nego_request);
  NegoAResponse a_nego_response;
  if (!bob.OnNegoRequest(a_nego_request, a_nego_response)) {
    assert(false);
    return false;
  }
  if (!alice.OnNegoResponse(a_nego_response)) {
    assert(false);
    return false;
  }

  Request request;
  bob.GetRequest(request);

  Response response;
  if (!alice.OnRequest(std::move(request), response)) {
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
    Claim claim;
    if (!bob.GenerateClaim(claim)) {
      assert(false);
      return false;
    }
    std::cout << "claim: " << claim.i << "," << claim.j << "\n";
    if (!VerifyClaim(alice_data->bulletin().s, receipt, secret, claim)) {
      assert(false);
      return false;
    }
    std::cout << "verify claim success\n";
  }

  return true;
}

}  // namespace scheme::ot_complaint

namespace scheme::plain::ot_complaint {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, std::vector<Range> const& phantoms,
          bool test_evil) {
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
    return scheme::ot_complaint::Test(output_path, alice_data, bob_data,
                                      demands, phantoms, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}

}  // namespace scheme::plain::ot_complaint

namespace scheme::table::ot_complaint {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, std::vector<Range> const& phantoms,
          bool test_evil) {
  try {
    using scheme::table::AliceData;
    using scheme::table::BobData;
    auto alice_data = std::make_shared<AliceData>(publish_path);

    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto bob_data = std::make_shared<BobData>(bulletin_file, public_path);
    auto const& bulletin = bob_data->bulletin();
    std::cout << "n: " << bulletin.n << ", s: " << bulletin.s << "\n";
    return scheme::ot_complaint::Test(output_path, alice_data, bob_data,
                                      demands, phantoms, test_evil);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme::table::ot_complaint