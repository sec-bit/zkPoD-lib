#include "scheme_otbatch3_test.h"
#include "scheme_otbatch3_client.h"
#include "scheme_otbatch3_notary.h"
#include "scheme_otbatch3_protocol.h"
#include "scheme_otbatch3_session.h"
#include "scheme_plain.h"
#include "scheme_plain_a.h"
#include "scheme_plain_b.h"
#include "scheme_table.h"
#include "scheme_table_a.h"
#include "scheme_table_b.h"

namespace {
// The session id must be hash(addr_A), and the client id must be hash(addr_B).
// Here just use two dummy values for test.
const h256_t kDummySessionId = h256_t{{1}};
const h256_t kDummyClientId = h256_t{{2}};
}  // namespace

namespace scheme::otbatch3 {

template <typename A, typename B>
bool Test(std::string const& output_path, std::shared_ptr<A> a,
          std::shared_ptr<B> b, std::vector<Range> const& demands,
          std::vector<Range> const& phantoms) {
  Tick _tick_(__FUNCTION__);

  auto output_file = output_path + "/decrypted_data";

  Session<A> session(a, kDummySessionId, kDummyClientId);
  Client<B> client(b, kDummyClientId, kDummySessionId, demands, phantoms);

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
  if (!session.OnRequest(std::move(request), response)) {
    assert(false);
    return false;
  }

  Receipt receipt;
  if (!client.OnResponse(std::move(response), receipt)) {
    assert(false);
    return false;
  }

  Secret secret;
  if (!session.OnReceipt(receipt, secret)) {
    assert(false);
    return false;
  }

  if (!client.OnSecret(secret)) {
    assert(false);
    return false;
  }

  if (!client.SaveDecrypted(output_file)) {
    assert(false);
    return false;
  }

  std::cout << "success: save to " << output_file << "\n";
  return true;
}
}  // namespace scheme::otbatch3

namespace scheme::plain::otbatch3 {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands,
          std::vector<Range> const& phantoms) {
  try {
    using scheme::plain::A;
    using scheme::plain::B;
    auto a = std::make_shared<A>(publish_path);
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    auto const& bulletin = b->bulletin();
    std::cout << "n: " << bulletin.n << ", s: " << bulletin.s
              << ", size: " << bulletin.size << "\n";
    return scheme::otbatch3::Test(output_path, a, b, demands, phantoms);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme::plain::otbatch3

namespace scheme::table::otbatch3 {

bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands,
          std::vector<Range> const& phantoms) {
  try {
    using scheme::table::A;
    using scheme::table::B;
    auto a = std::make_shared<A>(publish_path);
    std::string bulletin_file = publish_path + "/bulletin";
    std::string public_path = publish_path + "/public";
    auto b = std::make_shared<B>(bulletin_file, public_path);
    auto const& bulletin = b->bulletin();
    std::cout << "n: " << bulletin.n << ", s: " << bulletin.s << "\n";
    return scheme::otbatch3::Test(output_path, a, b, demands, phantoms);
  } catch (std::exception& e) {
    std::cerr << __FUNCTION__ << "\t" << e.what() << "\n";
    return false;
  }
}
}  // namespace scheme::table::otbatch3