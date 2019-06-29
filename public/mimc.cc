#include "mimc.h"

std::vector<Fr> MimcConst(std::string const& prefix, size_t count) {
  std::vector<Fr> ret(count);
  for (size_t i = 0; i < count; ++i) {
    h256_t digest;
    CryptoPP::Keccak_256 hash;
    std::string s = prefix + std::to_string(i);
    hash.Update((uint8_t const*)s.data(), s.size());
    hash.Final(digest.data());

    bool success;
    ret[i].setArray(&success, digest.data(), digest.size(), mcl::fp::Mod);
    assert(success);
  }
  return ret;
}

std::vector<Fr> const& Mimc3Const() {
  static std::vector<Fr> kConst(MimcConst("mimc_3_const", kMimc3Round));
  return kConst;
}

std::vector<Fr> const& MimcInvConst() {
  static std::vector<Fr> kConst(MimcConst("mimc_inv_const", kMimcInvRound));
  return kConst;
}

Fr Mimc3(Fr const& left, Fr const& right) {
  auto const& kConst = Mimc3Const();
  std::vector<Fr> x(kConst.size());
  auto box = [](Fr const& v) { return v * v * v; };
  x[0] = right + box(left + kConst[0]);
  x[1] = left + box(x[0] + kConst[1]);
  for (size_t i = 2; i < kConst.size(); ++i) {
    x[i] = x[i - 2] + box(x[i - 1] + kConst[i]);
  }
  return x.back();
}

Fr Mimc3Circuit(Fr const& left, Fr const& right) {
  auto const& kConst = Mimc3Const();
  std::vector<Fr> x(kConst.size());
  std::vector<Fr> y(kConst.size());

  x[0] = (left + kConst[0]) * (left + kConst[0]);
  y[0] = right + (left + kConst[0]) * x[0];

  x[1] = (y[0] + kConst[1]) * ((y[0] + kConst[1]));
  y[1] = left + ((y[0] + kConst[1])) * x[1];

  for (size_t i = 2; i < kConst.size(); ++i) {
    x[i] = (y[i - 1] + kConst[i]) * (y[i - 1] + kConst[i]);
    y[i] = y[i - 2] + (y[i - 1] + kConst[i]) * x[i];
  }
  return y.back();
}

Fr MimcInv(Fr const& s) {
  auto const& kConst = MimcInvConst();
  std::vector<Fr> x(kConst.size());
  auto box = [](Fr const& v) { return v.inverse(); };
  x[0] = box(s + kConst[0]);
  x[1] = s + box(x[0] + kConst[1]);
  for (size_t i = 2; i < kConst.size(); ++i) {
    x[i] = x[i - 2] + box(x[i - 1] + kConst[i]);
  }
  return x.back();
}

Fr MimcInvCircuit(Fr const& s) {
  auto const& kConst = MimcInvConst();
  std::vector<Fr> x(kConst.size());
  x[0] = FrInv(s + kConst[0]);
  x[1] = s + FrInv(x[0] + kConst[1]);
  for (size_t i = 2; i < kConst.size(); ++i) {
    x[i] = x[i - 2] + FrInv(x[i - 1] + kConst[i]);
  }
  return x.back();
}