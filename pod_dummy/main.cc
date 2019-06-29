#include <assert.h>
#include <cryptopp/osrng.h>
#include <cryptopp/randpool.h>
#include <iostream>
#include "bp.h"
#include "ecc.h"
#include "misc.h"
#include "tick.h"

Fr FrPower2(Fr const& base, mpz_class const& exp) {
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

  auto e = exp.get_mpz_t();
  size_t n = mpz_sizeinbase(e, 2);
  std::vector<Fr> exp_bits(n);
  for (size_t i = 0; i < n; ++i) {
    exp_bits[i] = mpz_tstbit(e, i) ? Fr(1) : Fr(0);
  }

  std::vector<Fr> exp_bit_base(n);
  for (size_t i = 0; i < n; ++i) {
    exp_bit_base[i] = Fr(1) - exp_bits[i] + exp_bits[i] * base;
  }

  std::vector<Fr> result2(n - 1);
  std::vector<Fr> result(n - 2);
  Fr out;

  for (size_t i = 0; i < n - 1; ++i) {
    auto& r_1 = i == 0 ? exp_bit_base[n - 1] : result[i - 1];
    result2[i] = r_1 * r_1;
    auto& r = i == (n - 2) ? out : result[i];
    r = result2[i] * exp_bit_base[n - i - 2];
  }
  return out;
}

Fr FrPower3(Fr const& base, mpz_class const& exp) {
  auto e = exp.get_mpz_t();
  size_t n = mpz_sizeinbase(e, 2);
  std::vector<Fr> exp_bits(n);
  for (size_t i = 0; i < n; ++i) {
    exp_bits[i] = mpz_tstbit(e, i) ? Fr(1) : Fr(0);
  }

  std::vector<Fr> base_pow2(n);  // base^(2^i)
  base_pow2[0] = base;
  for (size_t i = 1; i < n; ++i) {
    base_pow2[i] = base_pow2[i - 1] * base_pow2[i - 1];
  }

  std::vector<Fr> result2(n);
  for (size_t i = 0; i < n; ++i) {
    result2[i] = Fr(1) - exp_bits[i] + exp_bits[i] * base_pow2[i];
  }

  std::vector<Fr> result(n);
  result[0] = result2[0];
  for (size_t i = 1; i < n; ++i) {
    result[i] = result[i - 1] * result2[i];
  }
  return result[n - 1];
}

void test() {
  for (int i = 0; i < 100; ++i) {
    Fr base = FrRand();
    mpz_class exp = misc::RandMpz32();
    auto a1 = FrPower(base, exp);
    auto a2 = FrPower2(base, exp);
    auto a3 = FrPower3(base, exp);
    assert(a1 == a2);
    assert(a3 == a2);
  }
}

bool TestBp() {
  size_t count = 1024 * 1024;
  std::vector<G1> g(count);
  for (auto& i : g) {
    i = G1Rand();
  }
  std::vector<Fr> f(count);
  for (auto& i : f) {
    i = FrRand();
  }

  bp::Challenge challenge(count, rand(), false);  // NOTE: use rand() for test

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

  bp::P1Proof p1_proof = bp::P1Prove(get_g, get_f, count);

  if (!bp::P1Verify(p1_proof, get_g, count)) {
    assert(false);
    return false;
  }

  std::vector<uint8_t> buf(p1_proof.GetBufSize());
  auto size = p1_proof.serialize(buf.data(), buf.size());
  if (size != buf.size()) {
    assert(false);
    return false;
  }

  bp::P1Proof p1_proof_temp;
  size = p1_proof_temp.deserialize(buf.data(), buf.size());
  if (!size) {
    assert(false);
    return false;
  }
  if (p1_proof_temp != p1_proof) {
    assert(false);
    return false;
  }

  std::cout << "success\n";
  return true;
}

Fr Mimc(std::vector<Fr> const& c, Fr const& seed, uint64_t offset) {
  Fr s = seed + offset;
  std::vector<Fr> x(c.size());

  auto pow_3 = [](Fr const& v) { return v * v * v; };

  x[0] = pow_3(s + c[0]);
  x[1] = s + pow_3(x[0] + c[1]);
  for (size_t i = 2; i < c.size(); ++i) {
    x[i] = x[i - 2] + pow_3(x[i - 1] + c[i]);
  }
  return x.back();
}

Fr Mimc2(std::vector<Fr> const& c, size_t round) {
  std::vector<Fr> x(round);
  x[0] = FrRand();
  for (size_t i = 1; i < round; ++i) {
    auto temp = x[i - 1] + c[i % c.size()];
    x[i] = FrInv(temp);
    //temp* temp* temp;
  }
  return x.back();
}

void TestMimc() {
  std::vector<Fr> c(323);
  for (auto& i : c) {
    i = FrRand();
  }

  Tick tick(__FUNCTION__);
  Fr ret = Mimc2(c, 8192000);

  std::cout << ret << "\n";
  // auto poly = PolyMimc(c, Fr::one());
}

int main(int /*argc*/, char** /*argv*/) {
  InitEcc();
  // TestBp();
  TestMimc();
  return 0;
}
