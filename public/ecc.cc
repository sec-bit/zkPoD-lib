#include "ecc.h"

#include <cryptopp/keccak.h>
#include <cryptopp/osrng.h>
#include <cryptopp/sha.h>

#include "ecc_pub.h"
#include "msvc_hack.h"

namespace {
// NOTE: NonblockingRng should enough for linux & windows
// thread_local CryptoPP::AutoSeededRandomPool rng;
thread_local CryptoPP::NonblockingRng rng;
}  // namespace

void InitEcc() {
  std::cout << "init mcl in main\n";
  mcl::bn::CurveParam cp = mcl::BN_SNARK1;
  mcl::bn256::initPairing(cp);
  // Fr::setIoMode(mcl::IoMode::IoSerialize);
  // G1::setIoMode(mcl::IoMode::IoSerialize);
  // G2::setIoMode(mcl::IoMode::IoSerialize);
}

void FpRand(Fp* f) {
  uint8_t buf[32];
  rng.GenerateBlock(buf, 32);
  f->setArrayMask(buf, 32);
}

Fp FpRand() {
  Fp r;
  FpRand(&r);
  return r;
}

Fp2 Fp2Rand() { return Fp2(FpRand(), FpRand()); }

void FrRand(Fr* f) {
  uint8_t buf[32];
  rng.GenerateBlock(buf, 32);
  f->setArrayMask(buf, 32);
}

Fr FrRand() {
  Fr r;
  FrRand(&r);
  return r;
}

void FrRand(Fr* r, size_t n) {
  std::vector<uint8_t> h(n * 32);

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (size_t i = 0; i < 4; ++i) {
    rng.GenerateBlock(h.data() + 8 * i * n, 8 * n);
  }

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (size_t i = 0; i < n; ++i) {
    r[i].setArrayMask(h.data() + i * 32, 32);
  }
}

void FrRand(std::vector<Fr*>& f) {
  auto n = f.size();
  std::vector<uint8_t> h(n * 32);
#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (size_t i = 0; i < 4; ++i) {
    rng.GenerateBlock(h.data() + 8 * i * n, 8 * n);
  }

#ifdef MULTICORE
#pragma omp parallel for
#endif
  for (size_t i = 0; i < f.size(); ++i) {
    f[i]->setArrayMask(h.data() + i * 32, 32);
  }
}

Fr FrInv(Fr const& r) {
  Fr r_inv;
  Fr::inv(r_inv, r);
  return r_inv;
}

void FrInv(Fr* begin, uint64_t count) {
  assert(count > 0);

  std::vector<Fr> prod(count);

  Fr acc(1);

  for (size_t i = 0; i < count; ++i) {
    assert(!begin[i].isZero());
    prod[i] = acc;
    acc *= begin[i];
  }

  Fr acc_inverse = FrInv(acc);

  for (int64_t i = (int64_t)count - 1; i >= 0; --i) {
    Fr old_el = begin[i];
    begin[i] = acc_inverse * prod[i];
    acc_inverse *= old_el;
  }
}

void FrInv(std::vector<Fr>& vec) { FrInv(vec.data(), vec.size()); }

G1 G1Rand() {
  G1 out;
  bool b;
  for (;;) {
    mcl::bn256::mapToG1(&b, out, FpRand());
    if (b) break;
  }
  return out;
}

void G1Rand(G1* r, size_t n) {
  bool b;
  Fp f;
  for (size_t i = 0; i < n; ++i) {
    for (;;) {
      FpRand(&f);
      mcl::bn256::mapToG1(&b, r[i], f);
      if (b) break;
    }
  }
}

G2 G2Rand() {
  G2 out;
  bool b;
  for (;;) {
    mcl::bn256::mapToG2(&b, out, Fp2Rand());
    if (b) break;
  }
  return out;
}

Fr FrZero() { return Fr(0); }

Fr FrOne() { return Fr(1); }

G1 G1Zero() {
  G1 r;
  r.clear();
  return r;
}

void G1Zero(G1* r, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    r[i].clear();
  }
}

G1 G1One() {
  G1 r(Fp(1), Fp(2));
  return r;
}

G2 G2Zero() {
  G2 r;
  r.clear();
  return r;
}

void G2Zero(G2* r, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    r[i].clear();
  }
}

G2 G2One() {
  G2 r(Fp2("1085704699902305713594457076223282948137075635957851808699051999328"
           "5655852781",
           "1155973203298638710799100402139228578392581286182119253091740315145"
           "2391805634"),
       Fp2("8495653923123431417604973247489272438418190587263600148770280649306"
           "958101930",
           "4082367875863433681332203403145435568316851327593401208105741076214"
           "120093531"));
  return r;
}

G1 MapToG1(Fp t) {
  G1 r;
  bool b;
  for (;;) {
    mcl::bn256::mapToG1(&b, r, t);
    if (b) break;
    t = t + 1;
  }
  return r;
}

G1 MapToG1(void const* b, size_t n) {
  G1 r;
  mcl::bn256::hashAndMapToG1(r, b, n);
  return r;
}

G1 MapToG1(std::string const& s) {
  G1 r;
  mcl::bn256::hashAndMapToG1(r, s.data(), s.size());
  return r;
}

G2 MapToG2(Fp2 t) {
  G2 r;
  bool b;
  for (;;) {
    mcl::bn256::mapToG2(&b, r, t);
    if (b) break;
    t = t + 1;
  }
  return r;
}

G2 MapToG2(void const* b, size_t n) {
  G2 r;
  mcl::bn256::hashAndMapToG2(r, b, n);
  return r;
}

G2 MapToG2(std::string const& s) {
  G2 r;
  mcl::bn256::hashAndMapToG2(r, s.data(), s.size());
  return r;
}

// inner product
G1 MultiExp(G1 const* g, Fr const* f, size_t n) {
  G1 r = G1Zero();
  for (size_t i = 0; i < n; ++i) {
    r += g[i] * f[i];
  }
  return r;
}

// a * g + b * g
G1 MultiExp(G1 const& g, Fr const& a, G1 const& h, Fr const& b) {
  return g * a + h * b;
}

G1 MultiExp(G1 const* g, Fr const* a, G1 const* h, Fr const* b, size_t n) {
  auto r1 = MultiExp(g, a, n);
  auto r2 = MultiExp(h, b, n);
  return r1 + r2;
}

Fr InnerProduct(Fr const* a, Fr const* b, size_t n) {
  Fr ret = FrZero();
  for (size_t i = 0; i < n; ++i) {
    ret += a[i] * b[i];
  }
  return ret;
}

Fr InnerProduct(std::function<Fr(size_t)> const& get_a,
                std::function<Fr(size_t)> const& get_b, size_t n) {
  Fr ret = FrZero();
  for (size_t i = 0; i < n; ++i) {
    ret += get_a(i) * get_b(i);
  }
  return ret;
}

Fr StrHashToFr(std::string const& s) {
  Fr f;
  f.setHashOf(s.data(), s.size());
  return f;
}

// because the Fr is 254bits, so we only use 31bytes
Fr BinToFr31(void const* start, void const* end) {
  assert((end > start) && ((uint8_t*)end - (uint8_t*)start) <= 31);
  uint8_t buf[32];
  size_t len = (uint8_t*)end - (uint8_t*)start;
  len = std::min<size_t>(len, 31);
  memcpy(buf, start, len);
  memset(buf + len, 0, sizeof(buf) - len);

  Fr fr;
  fr.deserialize(buf, 32);

  return fr;
}

// the buffer of start must >= 32
bool BinToFr32(void const* start, Fr* fr) {
  return fr->deserialize(start, 32) == 32;
}

Fr BinToFr32(void const* start) {
  Fr r;
  if (BinToFr32(start, &r)) return r;
  throw std::runtime_error("");
}

// buf must 32 bytes, if the fr comes from BinToFr31(), the buf[31] will be 0
void FrToBin(Fr const& fr, uint8_t* buf) {
  if (fr.serialize(buf, 32) != 32) throw std::runtime_error("");
}

h256_t FrToBin(Fr const& fr) {
  h256_t ret;
  FrToBin(fr, ret.data());
  return ret;
}

bool StrToFr(std::string const& s, Fr* fr) {
  try {
    fr->setStr(s);
    return true;
  } catch (std::exception&) {
    return false;
  }
}

// may throw exception
Fr StrToFr(std::string const& s) {
  Fr fr;
  fr.setStr(s);
  return fr;
}

std::string FrToStr(Fr const& fr) { return fr.getStr(); }

// buf must 32 bytes
bool BinToG1(uint8_t const* buf, G1* g) {
  return g->deserialize(buf, 32) == 32;
}

G1 BinToG1(uint8_t const* buf) {
  G1 r;
  if (BinToG1(buf, &r)) return r;
  throw std::runtime_error("");
}

// buf must 32 bytes
void G1ToBin(G1 const& g, uint8_t* buf) {
  if (g.serialize(buf, 32) != 32) throw std::runtime_error("");
}

h256_t G1ToBin(G1 const& g) {
  h256_t r;
  G1ToBin(g, r.data());
  return r;
}

bool StrToG1(std::string const& s, G1* g) {
  try {
    g->setStr(s);
    return true;
  } catch (std::exception&) {
    return false;
  }
}

// may throw
G1 StrToG1(std::string const& s) {
  G1 g;
  g.setStr(s);
  return g;
}

std::string G1ToStr(G1 const& g) { return g.getStr(); }

// buf must 64 bytes
bool BinToG2(uint8_t const* buf, G2* g) {
  return g->deserialize(buf, 64) == 64;
}

G2 BinToG2(uint8_t const* buf) {
  G2 r;
  if (BinToG2(buf, &r)) return r;
  throw std::runtime_error("");
}

// buf must 64 bytes
void G2ToBin(G2 const& g, uint8_t* buf) {
  if (g.serialize(buf, 64) != 64) throw std::runtime_error("");
}

bool StrToG2(std::string const& s, G2* g) {
  try {
    g->setStr(s);
    return true;
  } catch (std::exception&) {
    return false;
  }
}

// may throw exception
G2 StrToG2(std::string const& s) {
  G2 g;
  g.setStr(s);
  return g;
}

std::string G2ToStr(G2 const& g) { return g.getStr(); }

// very fast if fr is uint64/uint32
G1 MultiExpBosCoster(std::function<G1(size_t)> const& get_g,
                     std::function<Fr(size_t)> const& get_f, size_t n) {
  if (n == 0) return G1Zero();
  if (n == 1) return get_g(0) * get_f(0);

  class OrderedExponent {
   public:
    size_t idx() const { return idx_; }
    Fr const& fr() const { return fr_; }
    size_t num_bits() const { return num_bits_; }
    OrderedExponent(size_t idx, Fr const& fr) : idx_(idx), fr_(fr) {
      num_bits_ = NumOfBits();
    }
    bool operator<(OrderedExponent const& other) const {
      return fr_ < other.fr_;
    }
    void Minus(Fr const& b) {
      fr_ -= b;
      num_bits_ = NumOfBits();
    }
    void Clear() { fr_.clear(); }

   private:
    size_t NumOfBits() const {
      auto mpz = fr_.getMpz();
      return mpz_sizeinbase(mpz.get_mpz_t(), 2);
    }

   private:
    size_t idx_;
    Fr fr_;
    size_t num_bits_;
  };

  std::vector<OrderedExponent> opt_q;
  size_t const odd_n = (n % 2) ? n : (n + 1);
  opt_q.reserve(odd_n);
  std::vector<G1> g;
  g.reserve(odd_n);
  for (size_t i = 0; i < n; ++i) {
    g.emplace_back(get_g(i));
    opt_q.emplace_back(OrderedExponent(i, get_f(i)));
  }
  std::make_heap(opt_q.begin(), opt_q.end());
  if (n != odd_n) {
    g.emplace_back(G1Zero());
    opt_q.emplace_back(OrderedExponent(odd_n - 1, FrZero()));
  }

  G1 opt_result = G1Zero();

  for (;;) {
    OrderedExponent& a = opt_q[0];
    OrderedExponent& b = (opt_q[1] < opt_q[2] ? opt_q[2] : opt_q[1]);

    const size_t abits = a.num_bits();

    if (b.fr().isZero()) {
      opt_result = opt_result + (g[a.idx()] * a.fr());
      break;
    }

    const size_t bbits = b.num_bits();
    const size_t limit = (abits - bbits >= 20 ? 20 : abits - bbits);

    if (bbits < ((size_t)1) << limit) {
      /*
        In this case, exponentiating to the power of a is cheaper than
        subtracting b from a multiple times, so let's do it directly
      */
      opt_result = opt_result + (g[a.idx()] * a.fr());
      a.Clear();
    } else {
      // x A + y B => (x-y) A + y (B+A)
      // mpn_sub_n(a.r.data, a.r.data, b.r.data, 4);
      a.Minus(b.fr());
      g[b.idx()] = g[b.idx()] + g[a.idx()];
    }

    // regardless of whether a was cleared or subtracted from we push it down,
    // then take back up

    /* heapify A down */
    size_t a_pos = 0;
    while (2 * a_pos + 2 < odd_n) {
      // this is a max-heap so to maintain a heap property we swap with the
      // largest of the two
      if (opt_q[2 * a_pos + 1] < opt_q[2 * a_pos + 2]) {
        std::swap(opt_q[a_pos], opt_q[2 * a_pos + 2]);
        a_pos = 2 * a_pos + 2;
      } else {
        std::swap(opt_q[a_pos], opt_q[2 * a_pos + 1]);
        a_pos = 2 * a_pos + 1;
      }
    }

    /* now heapify A up appropriate amount of times */
    while (a_pos > 0 && opt_q[(a_pos - 1) / 2] < opt_q[a_pos]) {
      std::swap(opt_q[a_pos], opt_q[(a_pos - 1) / 2]);
      a_pos = (a_pos - 1) / 2;
    }
  }

  return opt_result;
}

G1 MultiExpBosCoster(G1 const* pg, Fr const* pf, size_t n) {
  auto get_g = [pg](size_t i) { return pg[i]; };
  auto get_f = [pf](size_t i) { return pf[i]; };
  return MultiExpBosCoster(get_g, get_f, n);
}

// e(a, G2One()) == e(c, d)
bool PairingMatch(G1 const& a, G1 const& c, G2 const& d) {
  auto const& ecc_pub = GetEccPub();
  Fp12 e1;
  mcl::bn256::precomputedMillerLoop(e1, a, ecc_pub.g2_1_coeff());
  mcl::bn256::finalExp(e1, e1);
  Fp12 e2;
  mcl::bn256::pairing(e2, c, d);
  assert(e1 == e2);
  return e1 == e2;
}

// e(a, b) == e(c, d)
bool PairingMatch(G1 const& a, G2 const& b, G1 const& c, G2 const& d) {
  Fp12 e1, e2;
  mcl::bn256::pairing(e1, a, b);
  mcl::bn256::pairing(e2, c, d);
  assert(e1 == e2);
  return (e1 == e2);
}

Fr FrPower(Fr const& base, mpz_class const& exp) {
  Fr z;
  Fr::pow(z, base, exp);
  return z;
  // Fr result(1);
  // bool begin = false;
  // auto e = exp.get_mpz_t();
  // ssize_t n = mpz_sizeinbase(e, 2);
  // for (ssize_t i = n - 1; i >= 0; --i) {
  //  if (begin) {
  //    result = result * result;
  //  }

  //  if (mpz_tstbit(e, i)) {
  //    begin = true;
  //    result *= base;
  //  }
  //}
  // return result;
}

Fr MapToFr(void const* b, size_t n) {
  CryptoPP::Keccak_256 hash;
  h256_t digest;
  hash.Update((uint8_t const*)b, n);
  hash.Final(digest.data());
  return BinToFr31(digest.data(), digest.data() + 31);
}

Fr MapToFr(uint64_t b) {
  auto b_big = boost::endian::native_to_big(b);
  return MapToFr((uint8_t const*)&b_big, sizeof(b_big));
}

