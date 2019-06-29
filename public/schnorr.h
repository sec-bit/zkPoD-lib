#pragma once

#include "ecc.h"

namespace schnorr {
struct Sig {
  G1 r;
  Fr s;
};

inline void Sign(G1 const& g, Fr const& sk, std::string const& data, Sig* sig) {
  Fr k = FrRand();
  sig->r = g * k;
  Fr e = StrHashToFr(sig->r.x.getStr() + data);
  sig->s = k + sk * e;
}

inline bool Verify(G1 const& g, G1 const& pk, std::string const& data,
                   Sig const& sig) {
  Fr e = StrHashToFr(sig.r.x.getStr() + data);
  return sig.r + pk * e == g * sig.s;  // two times ecc exp
}

}  // namespace schnorr
