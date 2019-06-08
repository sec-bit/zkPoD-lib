#pragma once

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100)
#pragma warning(disable : 4101)
#pragma warning(disable : 4702)
#endif
#include <yas/binary_iarchive.hpp>
#include <yas/binary_oarchive.hpp>
#include <yas/file_streams.hpp>
#include <yas/json_iarchive.hpp>
#include <yas/json_oarchive.hpp>
#include <yas/mem_streams.hpp>
#include <yas/serialize.hpp>
#include <yas/std_types.hpp>
#include <yas/types/std/array.hpp>
#include <yas/types/std/pair.hpp>
#include <yas/types/std/string.hpp>
#include <yas/types/std/vector.hpp>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include "basic_types.h"
#include "ecc.h"
#include "misc.h"

inline constexpr size_t YasBinF() {
  return yas::options::binary | yas::options::ebig | yas::options::compacted;
}

// save
template <typename Ar>
void serialize(Ar &ar, Range const &t) {
  ar &t.start;
  ar &t.count;
}

// load
template <typename Ar>
void serialize(Ar &ar, Range &t) {
  ar &t.start;
  ar &t.count;
}

// save
template <typename Ar>
void serialize(Ar &ar, h256_t const &t) {
  if (ar.type() == yas::binary) {
    ar &t.to_array();
  } else {
    assert(ar.type() == yas::json);
    auto str = misc::HexToStr(t);
    ar &str;
  }
}

// load
template <typename Ar>
void serialize(Ar &ar, h256_t &t) {
  if (ar.type() == yas::binary) {
    ar &t.to_array();
  } else {
    assert(ar.type() == yas::json);
    std::string str;
    ar &str;
    misc::HexStrToH256(str, t);
  }
}

namespace mcl {
// save
template <typename Ar>
void serialize(Ar &ar, G1 const &t) {
  if (ar.type() == yas::binary) {
    h256_t bin = G1ToBin(t);
    ar &bin;
  } else {
    assert(ar.type() == yas::json);
    std::string str = G1ToStr(t);
    ar &str;
  }
}

// load
template <typename Ar>
void serialize(Ar &ar, G1 &t) {
  if (ar.type() == yas::binary) {
    h256_t bin;
    ar &bin;
    t = BinToG1(bin.data());  // throw
  } else {
    assert(ar.type() == yas::json);
    std::string str;
    ar &str;
    t = StrToG1(str);  // throw
  }
}

// save
template <typename Ar>
void serialize(Ar &ar, G2 const &t) {
  if (ar.type() == yas::binary) {
    std::array<uint8_t, 64> bin;
    G2ToBin(t, bin.data());
    ar &bin;
  } else {
    assert(ar.type() == yas::json);
    std::string str = G2ToStr(t);
    ar &str;
  }
}

// load
template <typename Ar>
void serialize(Ar &ar, G2 &t) {
  if (ar.type() == yas::binary) {
    std::array<uint8_t, 64> bin;
    ar &bin;
    t = BinToG2(bin.data());  // throw
  } else {
    assert(ar.type() == yas::json);
    std::string str;
    ar &str;
    t = StrToG2(str);  // throw
  }
}

// save
template <typename Ar>
void serialize(Ar &ar, Fr const &t) {
  if (ar.type() == yas::binary) {
    h256_t bin = FrToBin(t);
    ar &bin;
  } else {
    assert(ar.type() == yas::json);
    std::string str = FrToStr(t);
    ar &str;
  }
}

// load
template <typename Ar>
void serialize(Ar &ar, Fr &t) {
  if (ar.type() == yas::binary) {
    h256_t bin;
    ar &bin;
    t = BinToFr32(bin.data());  // throw
  } else {
    assert(ar.type() == yas::json);
    std::string str;
    ar &str;
    t = StrToFr(str);  // throw
  }
}
}  // namespace mcl
