#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <memory>
#include <vector>

#include <boost/endian/conversion.hpp>
#include <boost/noncopyable.hpp>

#include "ecc.h"
#include "tick.h"

// Order of the G1 can get through Fr::getModulo(), which return
// 21888242871839275222246405745257275088548364400416034343698204186575808495617
// It's a prime number, that means for any generator u = g^xx, order of the sub
// group is same.
class EccPub : boost::noncopyable {
 public:
  G1WM const& g1_wm() const { return g1_wm_; }
  G2WM const& g2_wm() const { return g2_wm_; }
  std::vector<G1> const& u1() const { return u1_; }
  std::vector<G1WM> const& u1_wm() const { return u1_wm_; }
  std::vector<G2> const& u2() const { return u2_; }
  std::vector<G2WM> const& u2_wm() const { return u2_wm_; }
  std::vector<Fp6> const& g2_1_coeff() const { return g2_1_coeff_; }
  size_t u1_size() const { return u1_.size(); }
  size_t u2_size() const { return u2_.size(); }

  EccPub(std::string const& file) {
    LoadInternal(file);
    mcl::bn256::precomputeG2(g2_1_coeff_, G2One());
  }

  EccPub(uint64_t u1_size, uint64_t u2_size) {
    if (!u1_size || !u2_size)
      throw std::runtime_error("u1_size and u2_size can not be 0");

    Create(u1_size, u2_size);
    mcl::bn256::precomputeG2(g2_1_coeff_, G2One());
  }

  bool Save(std::string const& file) {
    try {
      SaveInternal(file);
      return true;
    } catch (std::exception& e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  }

  G1 PowerG1(Fr const& f) const {
    G1 ret;
    g1_wm_.mul(ret, f);
    return ret;
  }

  G2 PowerG2(Fr const& f) const {
    G2 ret;
    g2_wm_.mul(ret, f);
    return ret;
  }

  G1 PowerU1(uint64_t u_index, Fr const& f) const {
    if (u_index >= u1_wm_.size()) throw std::runtime_error("bad u_index");
    G1WM const& wm = u1_wm_[u_index];
    G1 ret;
    wm.mul(ret, f);
    return ret;
  }

  G2 PowerU2(uint64_t u_index, Fr const& f) const {
    if (u_index >= u2_wm_.size()) throw std::runtime_error("bad u_index");
    G2WM const& wm = u2_wm_[u_index];
    G2 ret;
    wm.mul(ret, f);
    return ret;
  }

 private:
  struct Header {
    uint64_t u1_size;
    uint64_t u2_size;
    uint64_t g1wm_size;
    uint64_t g2wm_size;
    uint64_t u1wm_size;
    uint64_t u2wm_size;
  };

  void Create(uint64_t u1_size, uint64_t u2_size) {
    Tick tick(__FUNCTION__);

    assert(u1_size && u2_size);
    //Fr::getOp().p;

    auto fr_bits = Fr::getBitSize();
    u1_.resize(u1_size);
    u1_wm_ = std::vector<G1WM>(u1_size);

    for (size_t i = 0; i < u1_.size(); ++i) {
      std::string seed = "pod_u1_" + std::to_string(i);
      u1_[i] = MapToG1(seed);
      u1_[i].normalize();

      u1_wm_[i].init(u1_[i], fr_bits, 4);  // use 4 is ok
    }

    u2_.resize(u2_size);
    u2_wm_ = std::vector<G2WM>(u2_size);

    for (size_t i = 0; i < u2_.size(); ++i) {
      std::string seed = "pod_u2_" + std::to_string(i);
      u2_[i] = MapToG2(seed);
      u2_[i].normalize();

      u2_wm_[i].init(u2_[i], fr_bits, 4);  // use 4 is ok
    }

    g1_wm_.init(G1One(), fr_bits, 8);  // use 8
    g2_wm_.init(G2One(), fr_bits, 8);
  }

  void SaveInternal(std::string const& file) {
    Tick tick(__FUNCTION__);
    FILE* f = fopen(file.c_str(), "wb+");
    if (!f) throw std::runtime_error("Create file failed");

    std::unique_ptr<FILE, decltype(&fclose)> auto_close(f, fclose);

    assert(!u1_.empty() && !u2_.empty());
    Header header;
    header.u1_size = u1_.size();
    header.u2_size = u2_.size();
    header.g1wm_size = GetG1wmSize(g1_wm_);
    header.g2wm_size = GetG2wmSize(g2_wm_);
    header.u1wm_size = GetG1wmSize(u1_wm_[0]);
    header.u2wm_size = GetG2wmSize(u2_wm_[0]);

    if (!WriteHeader(f, header)) {
      throw std::runtime_error("Write header failed");
    }

    if (!WriteG1wm(f, g1_wm_)) {
      throw std::runtime_error("Write g1_wm failed");
    }

    if (!WriteG2wm(f, g2_wm_)) {
      throw std::runtime_error("Write g2_wm failed");
    }

    for (auto& i : u1_) {
      if (!WriteG1(f, i)) {
        throw std::runtime_error("Write u1 failed");
      }
    }

    for (auto& i : u1_wm_) {
      if (!WriteG1wm(f, i)) {
        throw std::runtime_error("Write u1_wm failed");
      }
    }

    for (auto& i : u2_) {
      if (!WriteG2(f, i)) {
        throw std::runtime_error("Write u2 failed");
      }
    }

    for (auto& i : u2_wm_) {
      if (!WriteG2wm(f, i)) {
        throw std::runtime_error("Write u2_wm failed");
      }
    }
  }

  void LoadInternal(std::string const& file) {
    Tick tick(__FUNCTION__);
    FILE* f = fopen(file.c_str(), "rb");
    if (!f) throw std::runtime_error("Open failed");

    std::unique_ptr<FILE, decltype(&fclose)> auto_close(f, fclose);

    Header header;
    if (!ReadHeader(f, header)) throw std::runtime_error("Read header failed");
    if (!header.u1_size || !header.u2_size || !header.g1wm_size ||
        !header.g2wm_size || !header.u1wm_size || !header.u2wm_size)
      throw std::runtime_error("Invalid data");

    if (!ReadG1wm(f, header.g1wm_size, g1_wm_)) {
      throw std::runtime_error("Read g1_wm failed");
    }

    if (!ReadG2wm(f, header.g2wm_size, g2_wm_))
      throw std::runtime_error("Read g2_wm failed");

    u1_.resize(header.u1_size);
    for (auto& i : u1_) {
      if (!ReadG1(f, i)) throw std::runtime_error("Read u1 failed");
    }

    u1_wm_ = std::vector<G1WM>(header.u1_size);
    for (auto& i : u1_wm_) {
      if (!ReadG1wm(f, header.u1wm_size, i)) {
        throw std::runtime_error("Read u1_wm failed");
      }
    }

    u2_.resize(header.u2_size);
    for (auto& i : u2_) {
      if (!ReadG2(f, i)) throw std::runtime_error("Read u2 failed");
    }

    u2_wm_ = std::vector<G2WM>(header.u2_size);
    for (auto& i : u2_wm_) {
      if (!ReadG2wm(f, header.u2wm_size, i)) {
        throw std::runtime_error("Read u2_wm failed");
      }
    }
  }

 private:
  enum {
    kFpBufSize = 32,
    kG1BufSize = 1 + kFpBufSize * 2,
    kFp2BufSize = 64,
    kG2BufSize = 1 + kFp2BufSize * 2
  };

  static bool G1ToBinPlain(uint8_t* const buf, const G1* g) {
    if (g->isZero()) {
      memset(buf, 0, kG1BufSize);
      return true;
    }

    uint8_t* p = buf;
    G1 g_normalized;
    if (!g->isNormalized()) {
      g_normalized = *g;
      g_normalized.normalize();
      g = &g_normalized;
    }

    assert(g->z == 1);

    p[0] = 1;
    ++p;

    size_t n = g->x.serialize(p, kFpBufSize);
    p += kFpBufSize;
    assert(n);
    if (n == 0) return false;

    n = g->y.serialize(p, kFpBufSize);
    assert(n);
    return n > 0;
  }

  static bool BinToG1Plain(uint8_t const* const buf, G1* g) {
    if (buf[0] == 0) {
      g->clear();
      return true;
    }

    uint8_t const* p = buf;
    assert(p[0] == 1);
    g->z = 1;
    ++p;

    size_t n = g->x.deserialize(p, kFpBufSize);
    p += kFpBufSize;
    assert(n);
    if (n == 0) return false;

    n = g->y.deserialize(p, kFpBufSize);
    assert(n);
    return n > 0;
  }

  static bool G2ToBinPlain(uint8_t* const buf, const G2* g) {
    if (g->isZero()) {
      memset(buf, 0, kG2BufSize);
      return true;
    }

    uint8_t* p = buf;
    G2 g_normalized;
    if (!g->isNormalized()) {
      g_normalized = *g;
      g_normalized.normalize();
      g = &g_normalized;
    }

    assert(g->z == 1);

    p[0] = 1;
    ++p;

    size_t n = g->x.serialize(p, kFp2BufSize);
    p += kFp2BufSize;
    assert(n);
    if (n == 0) return false;

    n = g->y.serialize(p, kFp2BufSize);
    assert(n);
    return n > 0;
  }

  static bool BinToG2Plain(uint8_t const* const buf, G2* g) {
    if (buf[0] == 0) {
      g->clear();
      return true;
    }

    uint8_t const* p = buf;
    assert(p[0] == 1);
    g->z = 1;
    ++p;

    size_t n = g->x.deserialize(p, kFp2BufSize);
    p += kFp2BufSize;
    if (n == 0) return false;

    n = g->y.deserialize(p, kFp2BufSize);
    return n > 0;
  }

  static uint64_t GetG1wmSize(G1WM const& wm) {
    uint64_t size = sizeof(uint64_t) * 3;
    size += wm.tbl_.size() * kG1BufSize;
    return size;
  }

  static bool G1wmToBinPlain(G1WM const& wm, std::vector<uint8_t>& ret) {
    assert(wm.bitSize_ && wm.winSize_ && wm.tbl_.size());
    if (!wm.bitSize_ || !wm.winSize_ || !wm.tbl_.size()) return false;

    uint64_t bit_size = wm.bitSize_;
    auto bit_size_big = boost::endian::native_to_big(bit_size);

    uint64_t win_size = wm.winSize_;
    auto win_size_big = boost::endian::native_to_big(win_size);

    uint64_t tbl_size = wm.tbl_.size();
    auto tbl_size_big = boost::endian::native_to_big(tbl_size);

    uint64_t size = sizeof(uint64_t) * 3 + kG1BufSize * tbl_size;
    ret.resize(size);

    uint8_t* p = ret.data();

    memcpy(p, &bit_size_big, sizeof(bit_size_big));
    p += sizeof(bit_size_big);

    memcpy(p, &win_size_big, sizeof(win_size_big));
    p += sizeof(win_size_big);

    memcpy(p, &tbl_size_big, sizeof(tbl_size_big));
    p += sizeof(tbl_size_big);

    for (uint64_t i = 0; i < tbl_size; ++i) {
      G1 const& g = wm.tbl_[i];
      if (!G1ToBinPlain(p, &g)) return false;
      p += kG1BufSize;
    }

    assert(ret.size() == GetG1wmSize(wm));

    return true;
  }

  static bool BinToG1wmPlain(uint8_t const* buf, size_t len, G1WM& wm) {
    uint8_t const* p = buf;
    uint64_t left_len = len;

    if (len <= sizeof(uint64_t) * 3) return false;

    uint64_t bit_size;
    memcpy(&bit_size, p, sizeof(bit_size));
    p += sizeof(bit_size);
    bit_size = boost::endian::big_to_native(bit_size);
    wm.bitSize_ = bit_size;
    assert(wm.bitSize_);
    if (!wm.bitSize_) return false;

    uint64_t win_size;
    memcpy(&win_size, p, sizeof(win_size));
    p += sizeof(win_size);
    win_size = boost::endian::big_to_native(win_size);
    wm.winSize_ = win_size;
    assert(wm.winSize_);
    if (!wm.winSize_) return false;

    uint64_t tbl_size;
    memcpy(&tbl_size, p, sizeof(tbl_size));
    p += sizeof(tbl_size);
    tbl_size = boost::endian::big_to_native(tbl_size);
    assert(tbl_size);
    if (!tbl_size) return false;

    uint64_t check_tbl_size =
        (bit_size + win_size - 1) / win_size * ((uint64_t)1 << win_size);
    if (tbl_size != check_tbl_size) return false;

    left_len -= sizeof(uint64_t) * 3;
    if (left_len != kG1BufSize * tbl_size) return false;

    wm.tbl_.resize(tbl_size);
    for (uint64_t i = 0; i < tbl_size; ++i) {
      if (!BinToG1Plain(p, &wm.tbl_[i])) return false;
      p += kG1BufSize;
    }
    return true;
  }

  static uint64_t GetG2wmSize(G2WM const& wm) {
    uint64_t size = sizeof(uint64_t) * 3;
    size += wm.tbl_.size() * kG2BufSize;
    return size;
  }

  static bool G2wmToBinPlain(G2WM const& wm, std::vector<uint8_t>& ret) {
    assert(wm.bitSize_ && wm.winSize_ && wm.tbl_.size());
    if (!wm.bitSize_ || !wm.winSize_ || !wm.tbl_.size()) return false;

    uint64_t bit_size = wm.bitSize_;
    auto bit_size_big = boost::endian::native_to_big(bit_size);

    uint64_t win_size = wm.winSize_;
    auto win_size_big = boost::endian::native_to_big(win_size);

    uint64_t tbl_size = wm.tbl_.size();
    auto tbl_size_big = boost::endian::native_to_big(tbl_size);

    uint64_t size = sizeof(uint64_t) * 3 + kG2BufSize * wm.tbl_.size();
    ret.resize(size);

    uint8_t* p = ret.data();

    memcpy(p, &bit_size_big, sizeof(bit_size_big));
    p += sizeof(bit_size_big);

    memcpy(p, &win_size_big, sizeof(win_size_big));
    p += sizeof(win_size_big);

    memcpy(p, &tbl_size_big, sizeof(tbl_size_big));
    p += sizeof(tbl_size_big);

    for (uint64_t i = 0; i < tbl_size; ++i) {
      G2 const& g = wm.tbl_[i];
      if (!G2ToBinPlain(p, &g)) return false;
      p += kG2BufSize;
    }

    assert(ret.size() == GetG2wmSize(wm));

    return true;
  }

  static bool BinToG2wmPlain(uint8_t const* buf, size_t len, G2WM& wm) {
    uint8_t const* p = buf;
    uint64_t left_len = len;

    if (len <= sizeof(uint64_t) * 3) return false;

    uint64_t bit_size;
    memcpy(&bit_size, p, sizeof(bit_size));
    p += sizeof(bit_size);
    bit_size = boost::endian::big_to_native(bit_size);
    wm.bitSize_ = bit_size;
    assert(wm.bitSize_);
    if (!wm.bitSize_) return false;

    uint64_t win_size;
    memcpy(&win_size, p, sizeof(win_size));
    p += sizeof(win_size);
    win_size = boost::endian::big_to_native(win_size);
    wm.winSize_ = win_size;
    assert(wm.winSize_);
    if (!wm.winSize_) return false;

    uint64_t tbl_size;
    memcpy(&tbl_size, p, sizeof(tbl_size));
    p += sizeof(tbl_size);
    tbl_size = boost::endian::big_to_native(tbl_size);
    assert(tbl_size);
    if (!tbl_size) return false;

    uint64_t check_tbl_size =
        (bit_size + win_size - 1) / win_size * ((uint64_t)1 << win_size);
    if (tbl_size != check_tbl_size) return false;

    left_len -= sizeof(uint64_t) * 3;
    if (left_len != kG2BufSize * tbl_size) return false;

    wm.tbl_.resize(tbl_size);
    for (uint64_t i = 0; i < tbl_size; ++i) {
      if (!BinToG2Plain(p, &wm.tbl_[i])) return false;
      p += kG2BufSize;
    }
    return true;
  }

  template <typename T>
  static bool WriteUint(FILE* f, T v) {
    v = boost::endian::native_to_big(v);
    return fwrite(&v, sizeof(v), 1, f) == 1;
  }

  template <typename T>
  static bool ReadUint(FILE* f, T& v) {
    if (fread(&v, sizeof(v), 1, f) != 1) return false;
    v = boost::endian::big_to_native(v);
    return true;
  }

  static bool WriteHeader(FILE* f, Header const& v) {
    if (!WriteUint(f, v.u1_size)) return false;
    if (!WriteUint(f, v.u2_size)) return false;
    if (!WriteUint(f, v.g1wm_size)) return false;
    if (!WriteUint(f, v.g2wm_size)) return false;
    if (!WriteUint(f, v.u1wm_size)) return false;
    if (!WriteUint(f, v.u2wm_size)) return false;
    return true;
  }

  static bool ReadHeader(FILE* f, Header& v) {
    if (!ReadUint(f, v.u1_size)) return false;
    if (!ReadUint(f, v.u2_size)) return false;
    if (!ReadUint(f, v.g1wm_size)) return false;
    if (!ReadUint(f, v.g2wm_size)) return false;
    if (!ReadUint(f, v.u1wm_size)) return false;
    if (!ReadUint(f, v.u2wm_size)) return false;
    return true;
  }

  static bool WriteG1(FILE* f, G1 const& v) {
    uint8_t buf[kG1BufSize];
    if (!G1ToBinPlain(buf, &v)) return false;
    return fwrite(buf, kG1BufSize, 1, f) == 1;
  }

  static bool ReadG1(FILE* f, G1& v) {
    uint8_t buf[kG1BufSize];
    if (fread(buf, kG1BufSize, 1, f) != 1) return false;
    if (!BinToG1Plain(buf, &v)) return false;
    return true;
  }

  static bool WriteG2(FILE* f, G2 const& v) {
    uint8_t buf[kG2BufSize];
    if (!G2ToBinPlain(buf, &v)) return false;
    return fwrite(buf, kG2BufSize, 1, f) == 1;
  }

  static bool ReadG2(FILE* f, G2& v) {
    uint8_t buf[kG2BufSize];
    if (fread(buf, kG2BufSize, 1, f) != 1) return false;
    if (!BinToG2Plain(buf, &v)) return false;
    return true;
  }

  static bool WriteG1wm(FILE* f, G1WM const& v) {
    std::vector<uint8_t> bin;
    if (!G1wmToBinPlain(v, bin)) return false;
    return fwrite(bin.data(), bin.size(), 1, f) == 1;
  }

  static bool ReadG1wm(FILE* f, uint64_t size, G1WM& v) {
    std::unique_ptr<uint8_t[]> buf(new uint8_t[size]);
    if (fread(buf.get(), size, 1, f) != 1) return false;
    return BinToG1wmPlain(buf.get(), size, v);
  }

  static bool WriteG2wm(FILE* f, G2WM const& v) {
    std::vector<uint8_t> bin;
    if (!G2wmToBinPlain(v, bin)) return false;
    return fwrite(bin.data(), bin.size(), 1, f) == 1;
  }

  static bool ReadG2wm(FILE* f, uint64_t size, G2WM& v) {
    std::unique_ptr<uint8_t[]> buf(new uint8_t[size]);
    if (fread(buf.get(), size, 1, f) != 1) return false;
    return BinToG2wmPlain(buf.get(), size, v);
  }

 private:
  G1WM g1_wm_;
  G2WM g2_wm_;
  std::vector<G1> u1_;
  std::vector<G1WM> u1_wm_;
  std::vector<G2> u2_;
  std::vector<G2WM> u2_wm_;
  std::vector<Fp6> g2_1_coeff_;
};

inline EccPub& GetEccPub(std::string const& file = "") {
  static EccPub _instance_(file);
  return _instance_;
}

inline bool LoadEccPub(std::string const& file) {
  try {
    GetEccPub(file);
    return true;
  } catch (std::exception&) {
    return false;
  }
}

inline bool operator==(EccPub const& a, EccPub const& b) {
  if (a.g1_wm() != b.g1_wm()) return false;
  if (a.g2_wm() != b.g2_wm()) return false;
  if (a.u1() != b.u1()) return false;
  auto const& a_u1_wm = a.u1_wm();
  auto const& b_u1_wm = b.u1_wm();
  if (a_u1_wm.size() != b_u1_wm.size()) return false;
  for (size_t i = 0; i < a_u1_wm.size(); ++i) {
    if (a_u1_wm[i] != b_u1_wm[i]) return false;
  }

  if (a.u2() != b.u2()) return false;
  auto const& a_u2_wm = a.u2_wm();
  auto const& b_u2_wm = b.u2_wm();
  if (a_u2_wm.size() != b_u2_wm.size()) return false;
  for (size_t i = 0; i < a_u2_wm.size(); ++i) {
    if (a_u2_wm[i] != b_u2_wm[i]) return false;
  }

  return true;
}

inline bool operator!=(EccPub const& a, EccPub const& b) { return !(a == b); }
