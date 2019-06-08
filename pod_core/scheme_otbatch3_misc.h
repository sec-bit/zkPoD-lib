#pragma once

#include "misc.h"
#include "tick.h"
#include "ecc.h"

namespace scheme::otbatch3 {

struct RomChallenge {
  Fr c;
  Fr e1;
  Fr e2;
  Fr e1_square;
  Fr e2_square;
  Fr e1_e2_inverse;
};

struct W {
  W(RomChallenge const& challenge) {
    w1 = FrRand();
    w2 = FrRand();
    w1_w2 = w1 + w2;
    w1e1_w2e2 = w1 * challenge.e1 + w2 * challenge.e2;
    w1e1e1_w2e2e2 = w1 * challenge.e1_square + w2 * challenge.e2_square;
  }
  Fr w1;
  Fr w2;
  Fr w1_w2;
  Fr w1e1_w2e2;
  Fr w1e1e1_w2e2e2;
};

struct WN {
  WN(RomChallenge const& challenge, size_t count) {
    Tick _tick_(__FUNCTION__);
    w_.resize(count);
    for (auto& i : w_) {
      i.reset(new W(challenge));
    }
  }
  W const& w(size_t i) const { return *w_[i]; }

 private:
  std::vector<std::unique_ptr<W>> w_;
};

namespace details {
template <typename Bulletin, typename Request, typename Response>
h256_t ComputeSeed(h256_t const& client_id, h256_t const& session_id,
                   Bulletin const& bulletin, Request const& request,
                   Response const& response) {
  Tick _tick_(__FUNCTION__);
  CryptoPP::Keccak_256 hash;

  // client_id and session_id
  {
    hash.Update(client_id.data(), client_id.size());
    hash.Update(session_id.data(), session_id.size());
  }

  // bulletin
  {
    uint64_t big = bulletin.n;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));

    big = bulletin.s;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));

    // big = bulletin.size;
    // big = boost::endian::native_to_big(big);
    // hash.Update((uint8_t*)&big, sizeof(big));

    hash.Update(bulletin.sigma_mkl_root.data(), bulletin.sigma_mkl_root.size());
  }

  // request
  for (auto const& i : request.phantoms) {
    uint64_t big = i.start;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));
    big = i.count;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));
  }

  for (auto const& i : request.ot_vi) {
    auto bin = G1ToBin(i);
    hash.Update(bin.data(), bin.size());
  }

  {
    auto bin = G1ToBin(request.ot_v);
    hash.Update(bin.data(), bin.size());
  }

  // ot
  for (auto const& i : response.ot_ui) {
    auto bin = G1ToBin(i);
    hash.Update(bin.data(), bin.size());
  }

  // commitment of response
  for (auto const& i : response.uk) {
    for (auto const& j : i) {
      auto bin = G1ToBin(j);
      hash.Update(bin.data(), bin.size());
    }
  }
  for (auto const& i : response.ux0) {
    auto bin = G1ToBin(i);
    hash.Update(bin.data(), bin.size());
  }
  for (auto const& i : response.u0x) {
    for (auto const& j : i) {
      auto bin = G1ToBin(j);
      hash.Update(bin.data(), bin.size());
    }
  }
  for (auto const& i : response.g2x0) {
    std::array<uint8_t, 64> bin;
    G2ToBin(i, bin.data());
    hash.Update(bin.data(), bin.size());
  }
  for (auto const& i : response.ud) {
    auto bin = G1ToBin(i);
    hash.Update(bin.data(), bin.size());
  }
  {
    std::array<uint8_t, 64> bin;
    G2ToBin(response.g2d, bin.data());
    hash.Update(bin.data(), bin.size());
  }

  // result
  h256_t digest;
  hash.Final(digest.data());
  return digest;
}

void ComputeChallenge(h256_t const& seed, RomChallenge& challenge);
}  // namespace details

template <typename Bulletin, typename Request, typename Response>
void ComputeChallenge(RomChallenge& challenge, h256_t const& client_id,
                      h256_t const& session_id, Bulletin const& bulletin,
                      Request const& request, Response const& response) {
  auto seed =
      details::ComputeSeed(client_id, session_id, bulletin, request, response);
  details::ComputeChallenge(seed, challenge);
  std::cout << "challenge: " << challenge.c << "\n";
}
}  // namespace scheme::otbatch3