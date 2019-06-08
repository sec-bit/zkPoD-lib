#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include <cryptopp/keccak.h>
#include <boost/endian/conversion.hpp>

#include "ecc.h"
#include "mkl_tree.h"
#include "mpz.h"

inline G1 HashNameI(h256_t const& name, uint64_t i) {
  std::string s;
  s.reserve(64);
  s.assign((char*)name.data(), name.size());
  s += std::to_string(i);
  return MapToG1(s);
}

inline Fr Chain(uint8_t const* seed_buf, uint64_t seed_len, uint64_t index) {
  uint64_t index_be = boost::endian::native_to_big(index);

  h256_t digest_be;
  CryptoPP::Keccak_256 hash;
  hash.Update(seed_buf, seed_len);
  hash.Update((uint8_t const*)&index_be, sizeof(index_be));
  hash.Final(digest_be.data());
  
  // setArrayMaskMod want little endian
  h256_t digest_le;
  for (size_t i = 0; i < digest_be.size(); ++i) {
    digest_le.data()[i] = digest_be.data()[digest_be.size() - i - 1];
  }

  // use setArray(Mod) instead of setArrayMaskMod because of gas limit
  Fr r;  
  bool success = false;
  r.setArray(&success, digest_le.data(), digest_le.size(), mcl::fp::Mod);
  assert(success);
  //r.setArrayMaskMod(digest_le.data(), digest_le.size());

  return r;
}

inline Fr Chain(h256_t const& seed, uint64_t index) {
  return Chain(seed.data(), seed.size(), index);
}

inline uint32_t ChainUint32(h256_t const& seed, uint64_t index) {
  return (uint32_t)Chain(seed, index).getMpz().get_ui();
}

inline uint64_t ChainUint64(h256_t const& seed, uint64_t index) {
  return (uint64_t)Chain(seed, index).getMpz().get_ui();
}

inline size_t PackGCount(size_t count) {
  assert(count);
  auto align_count = mkl::Pow2UB(count);
  if (align_count == 1) return 1;
  return align_count / 2;
}

inline size_t PackXCount(size_t count) {
  return mkl::Log2UB(PackGCount(count));
}
