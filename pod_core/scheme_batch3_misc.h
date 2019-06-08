#pragma once

#include "scheme_otbatch3_misc.h"

namespace scheme::batch3 {
using RomChallenge = scheme::otbatch3::RomChallenge;
using W = scheme::otbatch3::W;
using WN = scheme::otbatch3::WN;

namespace details {
template <typename Bulletin, typename Request, typename Response>
h256_t ComputeSeed(h256_t const& client_id, h256_t const& session_id,
                   Bulletin const& bulletin, Request const& request,
                   Response const& response) {  
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

    hash.Update(bulletin.sigma_mkl_root.data(), bulletin.sigma_mkl_root.size());
  }

  // request
  for (auto const& i : request.demands) {
    uint64_t big = i.start;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));
    big = i.count;
    big = boost::endian::native_to_big(big);
    hash.Update((uint8_t*)&big, sizeof(big));
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
  Tick _tick_(__FUNCTION__);
  auto seed =
      details::ComputeSeed(client_id, session_id, bulletin, request, response);
  details::ComputeChallenge(seed, challenge);
  std::cout << "challenge: " << challenge.c << "\n";
}
}  // namespace scheme::batch3