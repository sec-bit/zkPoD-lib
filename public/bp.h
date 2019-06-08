#pragma once

#include <stdint.h>
#include <algorithm>
#include <functional>
#include <memory>

#include <boost/endian/conversion.hpp>

#include "ecc.h"
#include "multiexp.h"
#include "tick.h"
#include "misc.h"

namespace bp {

namespace detail {

inline size_t PackGCount(size_t count) {
  assert(count);
  auto align_count = misc::Pow2UB(count);
  if (align_count == 1) return 1;
  return align_count / 2;
}

inline size_t PackXCount(size_t count) {
  auto g_count = PackGCount(count);
  return misc::Log2UB(g_count);
}

inline G1 MultiExpGH(G1 const* g, Fr const* a, G1 const* h, Fr const* b,
                     size_t n) {
  auto get_g = [g, h, n](size_t i) -> G1 const& {
    return i < n ? g[i] : h[i - n];
  };
  auto get_f = [a, b, n](size_t i) -> Fr const& {
    return i < n ? a[i] : b[i - n];
  };
  return MultiExpBdlo12Inner<G1>(get_g, get_f, n * 2);
}
}  // namespace detail

class Challenge {
 public:
  Challenge(uint64_t count, uint64_t seed, bool zero_u) : count_(count) {
    auto seed_big = boost::endian::native_to_big(seed);
    Init((uint8_t const*)&seed_big, sizeof(seed_big), zero_u);
  }
  Challenge(uint64_t count, h256_t const& seed, bool zero_u) : count_(count) {
    Init(seed.data(), seed.size(), zero_u);
  }
  Challenge(uint64_t count, uint8_t const* seed, uint64_t size, bool zero_u)
      : count_(count) {
    Init(seed, size, zero_u);
  }

  uint64_t count() const { return count_; }
  std::vector<Fr> const& x() const { return x_; }
  std::vector<Fr> const& x_inverse() const { return x_inverse_; }
  std::vector<Fr> const& x_square() const { return x_square_; }
  std::vector<Fr> const& x_square_inverse() const { return x_square_inverse_; }
  G1 const& u() const { return u_; }
  bool zero_u() const { return u_ == G1Zero(); }

 private:
  void Init(uint8_t const* seed, uint64_t size, bool zero_u) {
    Tick tick(__FUNCTION__);
    using namespace detail;
    auto x_count = PackXCount(count_);
    x_.resize(x_count);
    x_inverse_.resize(x_count);
    x_square_.resize(x_count);
    x_square_inverse_.resize(x_count);

    std::string str_seed((char const*)seed, size);
    for (uint64_t i = 0; i < x_count; ++i) {
      auto str_x = str_seed + "_" + std::to_string(i) + "_x";
      x_[i] = StrHashToFr(str_x);
      Fr::inv(x_inverse_[i], x_[i]);
      Fr::sqr(x_square_[i], x_[i]);
      Fr::inv(x_square_inverse_[i], x_square_[i]);
    }

    if (zero_u) {
      u_ = G1Zero();
    } else {
      auto str_u = str_seed + "_u";
      u_ = MapToG1(str_u);
    }
  }

 private:
  uint64_t const count_;
  std::vector<Fr> x_;
  std::vector<Fr> x_inverse_;
  std::vector<Fr> x_square_;
  std::vector<Fr> x_square_inverse_;
  G1 u_;
};

struct P1Committment {
  G1 q(G1 const& u) const { return p + u * c; }
  G1 p;
  Fr c;

  bool operator==(P1Committment const& v) const { return p == v.p && c == v.c; }
  bool operator!=(P1Committment const& v) const { return !((*this) == v); }

  size_t GetBufSize() const { return 64; }

  size_t serialize(void* buf, uint64_t max_size) const {
    if (max_size < GetBufSize()) return 0;
    auto pbuf = (uint8_t*)buf;
    G1ToBin(p, pbuf);
    FrToBin(c, pbuf + 32);
    return 64;
  }

  size_t deserialize(void const* buf, uint64_t buf_size) {
    if (buf_size < GetBufSize()) return 0;
    auto pbuf = (uint8_t const*)buf;
    if (!BinToG1(pbuf, &p)) return 0;
    if (!BinToFr32(pbuf + 32, &c)) return 0;
    return 64;
  }
};

struct P2Proof {
  G1 q;
  std::vector<G1> left;  // size() == log(count)
  std::vector<G1> right;
  Fr a;
  Fr b;

  bool operator==(P2Proof const& v) const {
    return q == v.q && a == v.a && b == v.b && left == v.left &&
           right == v.right;
  }
  bool operator!=(P2Proof const& v) const { return !((*this) == v); }

  size_t GetBufSize() const {
    assert(left.size() == right.size());
    return 32 + 32 + 32 + sizeof(uint64_t) + 2 * 32 * left.size();
  }

  size_t serialize(void* buf, uint64_t max_size) const {
    assert(left.size() == right.size());

    size_t buf_size = GetBufSize();

    if (max_size < buf_size) return 0;

    auto pbuf = (uint8_t*)buf;

    G1ToBin(q, pbuf);
    pbuf += 32;

    FrToBin(a, pbuf);
    pbuf += 32;

    FrToBin(b, pbuf);
    pbuf += 32;

    uint64_t count = left.size();
    count = boost::endian::native_to_big(count);
    memcpy(pbuf, &count, sizeof(count));
    pbuf += sizeof(count);

    for (auto const& i : left) {
      G1ToBin(i, pbuf);
      pbuf += 32;
    }
    for (auto const& i : right) {
      G1ToBin(i, pbuf);
      pbuf += 32;
    }

    assert(buf_size == (uint64_t)(pbuf - (uint8_t*)buf));

    return buf_size;
  }

  size_t deserialize(void const* buf, uint64_t buf_size) {
    if (buf_size <= (32 + 32 + 32 + sizeof(uint64_t))) return 0;

    auto pbuf = (uint8_t const*)buf;

    if (!BinToG1(pbuf, &q)) return 0;
    pbuf += 32;

    if (!BinToFr32(pbuf, &a)) return 0;
    pbuf += 32;

    if (!BinToFr32(pbuf, &b)) return 0;
    pbuf += 32;

    uint64_t count;
    memcpy(&count, pbuf, sizeof(count));
    count = boost::endian::big_to_native(count);
    pbuf += sizeof(count);

    size_t data_size = 32 + 32 + 32 + sizeof(uint64_t) + 2 * 32 * count;
    if (buf_size < data_size) return 0;

    left.resize(count);
    for (auto& i : left) {
      if (!BinToG1(pbuf, &i)) return 0;
      pbuf += 32;
    }

    right.resize(count);
    for (auto& i : right) {
      if (!BinToG1(pbuf, &i)) return 0;
      pbuf += 32;
    }

    return buf_size;
  }
};

struct P1Proof {
  P1Committment committment;
  P2Proof p2_proof;

  bool operator==(P1Proof const& v) const {
    return committment == v.committment && p2_proof == v.p2_proof;
  }
  bool operator!=(P1Proof const& v) const { return !((*this) == v); }

  size_t GetBufSize() const {
    return committment.GetBufSize() + p2_proof.GetBufSize();
  }

  size_t serialize(void* buf, uint64_t max_size) const {
    auto pbuf = (uint8_t*)buf;
    auto size1 = committment.serialize(pbuf, max_size);
    if (!size1) return 0;
    auto size2 = p2_proof.serialize(pbuf + size1, max_size - size1);
    if (!size2) return 0;
    auto size = size1 + size2;
    assert(size == GetBufSize());
    return size;
  }

  size_t deserialize(void const* buf, uint64_t buf_size) {
    auto pbuf = (uint8_t const*)buf;
    auto size1 = committment.deserialize(pbuf, buf_size);
    if (!size1) return 0;
    auto size2 = p2_proof.deserialize(pbuf + size1, buf_size - size1);
    if (!size2) return 0;
    auto size = size1 + size2;
    assert(size == GetBufSize());
    return size;
  }
};

template <typename GET_G, typename GET_F>
P2Proof P2Prove(P1Committment const& p1_committment, GET_G const& get_g,
                GET_F const& get_f, Challenge const& challenge) {
  Tick tick(__FUNCTION__);
  using namespace detail;
  P2Proof p2_proof;
  uint64_t count = challenge.count();
  bool zero_u = challenge.zero_u();
  auto g_count = PackGCount(count);
  std::vector<Fr> a, b;
  a.reserve(g_count);
  b.reserve(g_count);
  for (size_t i = 0; i < g_count; ++i) {
    a.push_back(get_f(i));
    b.push_back(get_f(i + g_count));
  }

  std::vector<G1> g, h;
  g.reserve(g_count);
  h.reserve(g_count);
  for (size_t i = 0; i < g_count; ++i) {
    g.push_back(get_g(i));
    h.push_back(get_g(i + g_count));
  }

  p2_proof.q = p1_committment.q(challenge.u());

  G1 p = p2_proof.q;

  p2_proof.left.resize(challenge.x().size());
  p2_proof.right.resize(challenge.x().size());
  for (size_t loop = 0; loop < challenge.x().size(); ++loop) {
    auto const& x = challenge.x()[loop];
    auto const& x_inverse = challenge.x_inverse()[loop];
    auto const& x_square = challenge.x_square()[loop];
    auto const& x_square_inverse = challenge.x_square_inverse()[loop];

    auto nn = g.size() / 2;

    G1 L, R;
    if (nn >= (1024 * 32)) {
#pragma omp parallel sections
      {
#pragma omp section
        { L = MultiExpGH(&g[nn], &a[0], &h[0], &b[nn], nn); }

#pragma omp section
        { R = MultiExpGH(&g[0], &a[nn], &h[nn], &b[0], nn); }
      }
    } else {
      L = MultiExpGH(&g[nn], &a[0], &h[0], &b[nn], nn);
      R = MultiExpGH(&g[0], &a[nn], &h[nn], &b[0], nn);
    }

    if (!zero_u) {
      auto CL = InnerProduct(&a[0], &b[nn], nn);
      auto CR = InnerProduct(&a[nn], &b[0], nn);
      L += challenge.u() * CL;
      R += challenge.u() * CR;
    }

    std::vector<G1> gg;
    gg.resize(nn);
// very slow
#pragma omp parallel for
    for (int64_t i = 0; i < (int64_t)nn; ++i) {
      gg[i] = MultiExp(g[i], x_inverse, g[nn + i], x);
    }

    std::vector<G1> hh;
    hh.resize(nn);
// very slow
#pragma omp parallel for
    for (int64_t i = 0; i < (int64_t)nn; ++i) {
      hh[i] = MultiExp(h[i], x, h[nn + i], x_inverse);
    }

    auto pp = p + MultiExp(L, x_square, R, x_square_inverse);

    std::vector<Fr> aa;
    aa.resize(nn);
#pragma omp parallel for
    for (int64_t i = 0; i < (int64_t)nn; ++i) {
      aa[i] = a[i] * x + a[nn + i] * x_inverse;
    }

    std::vector<Fr> bb;
    bb.resize(nn);
#pragma omp parallel for
    for (int64_t i = 0; i < (int64_t)nn; ++i) {
      bb[i] = b[i] * x_inverse + b[nn + i] * x;
    }

    g.swap(gg);
    h.swap(hh);
    p = pp;
    a.swap(aa);
    b.swap(bb);

    p2_proof.left[loop] = L;
    p2_proof.right[loop] = R;
  }

  assert(g.size() == 1 && h.size() == 1);
  assert(a.size() == 1 && b.size() == 1);

  p2_proof.a = a[0];
  p2_proof.b = b[0];
  return p2_proof;
}

template <typename GET_G>
bool P2Verify(P1Committment const& p1_committment, GET_G const& get_g,
              P2Proof const& p2_proof, Challenge const& challenge) {
  Tick tick(__FUNCTION__);
  using namespace detail;
  uint64_t count = challenge.count();
  auto g_count = PackGCount(count);
  auto x_count = PackXCount(count);
  // std::cout << "proof.p: " << proof.p << std::endl;
  // std::cout << "proof.a: " << proof.a << std::endl;
  // std::cout << "proof.b: " << proof.b << std::endl;
  // std::cout << "challenge: " << challenge_seed << std::endl;

  auto const& u = challenge.u();
  G1 p = p1_committment.q(u);

  assert(p2_proof.left.size() == x_count && p2_proof.right.size() == x_count);
  if (p2_proof.left.size() != x_count || p2_proof.right.size() != x_count)
    return false;

  auto const& x = challenge.x();
  auto const& x_inverse = challenge.x_inverse();
  auto const& x_square = challenge.x_square();
  auto const& x_square_inverse = challenge.x_square_inverse();

  std::vector<G1> g, h;
  g.resize(g_count);
  h.resize(g_count);
  for (size_t i = 0; i < g_count; ++i) {
    g[i] = get_g(i);
    h[i] = get_g(i + g_count);
  }

  // true: 1; false: -1
  auto get_b = [x_count](size_t i, size_t j) -> bool {
    auto pow_j = (size_t)1 << (x_count - 1 - j);
    return (i & pow_j) ? true : false;
  };

  std::vector<Fr> ss;
  std::vector<Fr> ss_inverse;
  ss.resize(g_count);
  ss_inverse.resize(g_count);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)g_count; ++i) {
    ss[i] = Fr::one();
    for (size_t j = 0; j < x_count; ++j) {
      auto b = get_b(i, j);
      assert(!x[j].isZero());
      ss[i] = ss[i] * (b ? x[j] : x_inverse[j]);
    }
    Fr::inv(ss_inverse[i], ss[i]);
  }

  G1 last_g, last_h;

#pragma omp parallel sections
  {
#pragma omp section
    { last_g = MultiExpBdlo12(&g[0], &ss[0], g_count); }

#pragma omp section
    { last_h = MultiExpBdlo12(&h[0], &ss_inverse[0], g_count); }
  }

  G1 out = MultiExp(last_g, p2_proof.a, last_h, p2_proof.b);

  out += u * (p2_proof.a * p2_proof.b);

  if (x_count) {
    p += MultiExpGH(&p2_proof.left[0], &x_square[0], &p2_proof.right[0],
                    &x_square_inverse[0], x_count);
  }

  // std::cout << "out: " << out << std::endl;
  // std::cout << "p: " << p << std::endl;
  bool ret = out == p;
  assert(ret);
  return ret;
}

template <typename GET_G, typename GET_F>
P1Committment P1Commit(GET_G const& get_g, GET_F const& get_f, uint64_t count) {
  Tick tick(__FUNCTION__);
  using namespace detail;
  P1Committment p1_committment;
  auto g_count = PackGCount(count);
  p1_committment.c = InnerProduct(
      get_f, [&get_f, g_count](size_t i) { return get_f(i + g_count); },
      g_count);
  p1_committment.p = MultiExpBdlo12Inner<G1>(get_g, get_f, g_count * 2);
  return p1_committment;
}

template <typename GET_G, typename GET_F>
P1Proof P1Prove(GET_G const& get_g, GET_F const& get_f, uint64_t count) {
  P1Proof proof;
  proof.committment = P1Commit(get_g, get_f, count);
  Challenge challenge(count, G1ToBin(proof.committment.p), false);
  proof.p2_proof = P2Prove(proof.committment, get_g, get_f, challenge);

  return proof;
}

template <typename GET_G>
bool P1Verify(P1Committment const& p1_committment, GET_G const& get_g,
              P2Proof const& p2_proof, Challenge const& challenge) {
  if (!P2Verify(p1_committment, get_g, p2_proof, challenge)) return false;
  return p2_proof.q == p1_committment.q(challenge.u());
}

template <typename GET_G>
bool P1Verify(P1Proof const& p1_proof, GET_G const& get_g, uint64_t count) {
  Challenge challenge(count, G1ToBin(p1_proof.committment.p), false);
  if (!P2Verify(p1_proof.committment, get_g, p1_proof.p2_proof, challenge))
    return false;
  return p1_proof.p2_proof.q == p1_proof.committment.q(challenge.u());
}


inline bool Test() {
  size_t count = 1024;  // * 1024;
  std::vector<G1> g(count);
  for (auto& i : g) {
    i = G1Rand();
  }
  std::vector<Fr> f(count);
  for (auto& i : f) {
    i = FrRand();
  }

  Challenge challenge(count, rand(), false); // NOTE: use rand() for test

  auto get_g = [&g](uint64_t i) {
    if (i < g.size())
      return g[i];
    else
      return G1Zero();
  };

  auto get_f = [&f](uint64_t i) {
    if (i < f.size())
      return f[i];
    else
      return FrZero();
  };

  P1Proof p1_proof = P1Prove(get_g, get_f, count);

  if (!P1Verify(p1_proof, get_g, count)) {
    assert(false);
    return false;
  }

  std::vector<uint8_t> buf(p1_proof.GetBufSize());
  auto size = p1_proof.serialize(buf.data(), buf.size());
  if (size != buf.size()) {
    assert(false);
    return false;
  }

  P1Proof p1_proof_temp;
  size = p1_proof_temp.deserialize(buf.data(), buf.size());
  if (!size) {
    assert(false);
    return false;
  }
  if (p1_proof_temp != p1_proof) {
    assert(false);
    return false;
  }

  return true;
}
}  // namespace bp