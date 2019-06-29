#include "chain.h"
#include "mimc.h"
#include "mkl_tree.h"
#include "mpz.h"
#include "tick.h"

G1 HashNameI(h256_t const& name, uint64_t i) {
  std::string s;
  s.reserve(64);
  s.assign((char*)name.data(), name.size());
  s += std::to_string(i);
  return MapToG1(s);
}

Fr ChainKeccak256(uint8_t const* seed_buf, uint64_t seed_len, uint64_t index) {
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
  // r.setArrayMaskMod(digest_le.data(), digest_le.size());

  return r;
}

Fr ChainKeccak256(h256_t const& seed, uint64_t index) {
  return ChainKeccak256(seed.data(), seed.size(), index);
}

void ChainKeccak256(h256_t const& seed, uint64_t count, std::vector<Fr>& v) {
  Tick _tick_(__FUNCTION__);
  v.resize(count);

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (int64_t i = 0; i < (int64_t)count; ++i) {
    v[i] = ChainKeccak256(seed, i);
  }
}

Fr ChainMimcInv(Fr const& seed, uint64_t index) {
  return MimcInv(seed + index);
}

void ChainMimcInv(Fr const& seed, uint64_t count, std::vector<Fr>& v) {
  Tick _tick_(__FUNCTION__);
  v.resize(count);

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (uint64_t i = 0; i < count; ++i) {
    v[i] = ChainMimcInv(seed, i);
  }
}

// inline uint32_t ChainUint32(h256_t const& seed, uint64_t index) {
//  return (uint32_t)ChainKeccak256(seed, index).getMpz().get_ui();
//}
//
// inline uint64_t ChainUint64(h256_t const& seed, uint64_t index) {
//  return (uint64_t)ChainKeccak256(seed, index).getMpz().get_ui();
//}
//
