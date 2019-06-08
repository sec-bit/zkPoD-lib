#pragma once

#include <string>
#include <vector>

#include "ecc.h"

namespace scheme::otbatch3 {
struct NegoARequest {
  G2 s;
};

struct NegoAResponse {
  G2 s_exp_beta;
};

struct NegoBRequest {
  G1 t;
};

struct NegoBResponse {
  G1 t_exp_alpha;
};

struct Request {
  std::vector<Range> phantoms;  // sizeof() = L
  std::vector<G1> ot_vi;        // sizeof() = K
  G1 ot_v;
};

struct Response {
  // ot
  std::vector<G1> ot_ui;  // sizeof() = K

  // commitment
  std::vector<std::vector<G1>> uk;
  std::vector<G1> ux0;  // s
  std::vector<std::vector<G1>> u0x;
  std::vector<G2> g2x0;  // s
  std::vector<G1> ud;
  G2 g2d;
  // encrypted data
  std::vector<Fr> m;  // n*s
  // encrypted key
  std::vector<std::vector<Fr>> ek;
  std::vector<std::vector<Fr>> ex;
};

struct Receipt {
  G1 u0_x0_lgs;
  G1 u0d;
};

struct Secret {
  Fr x0_lgs;
  Fr d;
#ifdef _DEBUG
  std::vector<std::vector<Fr>> k;
  std::vector<std::vector<Fr>> x;
  std::vector<Fr> m;
#endif
};
}  // namespace scheme::otbatch3
