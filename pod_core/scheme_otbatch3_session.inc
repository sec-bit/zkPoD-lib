
namespace scheme::otbatch3 {

template <typename A>
Session<A>::Session(std::shared_ptr<A> a, h256_t const& self_id,
                    h256_t const& peer_id)
    : a_(a),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(a_->bulletin().n),
      s_(a_->bulletin().s),
      d_(FrRand()) {
  auto const& ecc_pub = GetEccPub();
  align_s_ = misc::Pow2UB(s_);
  log_s_ = misc::Log2UB(align_s_);
  if (align_s_ > ecc_pub.u1().size()) {
    assert(false);
    throw std::runtime_error("align_s too large");
  }
  ot_self_pk_ = G2Rand();
  ot_alpha_ = FrRand();
}

template <typename A>
void Session<A>::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(phantoms_count_);
  size_t index = 0;
  for (auto const& p : request_.phantoms) {
    for (size_t i = p.start; i < (p.start + p.count); ++i) {
      mappings_[index++].global_index = i;
    }
  }
}

template <typename A>
void Session<A>::GetNegoReqeust(NegoARequest& request) {
  request.s = ot_self_pk_;
}

template <typename A>
bool Session<A>::OnNegoRequest(NegoBRequest const& request,
                            NegoBResponse& response) {
  ot_peer_pk_ = request.t;
  response.t_exp_alpha = ot_peer_pk_ * ot_alpha_;
  return true;
}

template <typename A>
bool Session<A>::OnNegoResponse(NegoAResponse const& response) {
  ot_sk_ = response.s_exp_beta * ot_alpha_;
  return true;
}

template <typename A>
bool Session<A>::OnRequest(Request request, Response& response) {
  Tick _tick_(__FUNCTION__);

  request_ = std::move(request);

  if (!CheckPhantoms(n_, request_.phantoms)) {
    assert(false);
    return false;
  }

  for (auto const& i : request_.phantoms) {
    phantoms_count_ += i.count;
  }

  if (request_.ot_vi.size() >= phantoms_count_) {
    assert(false);
    return false;
  }

  BuildMapping();

  align_c_ = misc::Pow2UB(phantoms_count_);
  log_c_ = misc::Log2UB(align_c_);

  BuildOt(response.ot_ui);

  BuildK();
  BuildX();

  BuildUK(response.uk);
  BuildUX0(response.ux0);
  BuildU0X(response.u0x);
  BuildG2X0(response.g2x0);
  BuildCommitmentD(response.ud, response.g2d);

  ComputeChallenge(challenge_, peer_id_, self_id_, a_->bulletin(), request_,
                   response);

  BuildM(response.m);
  BlendM(response.m);
  BuildEK(response.ek);
  BuildEX(response.ex);

  return true;
}

template <typename A>
bool Session<A>::OnReceipt(Receipt const& receipt, Secret& secret) {
  Tick _tick_(__FUNCTION__);

  if (receipt.u0_x0_lgs != u0_x0_lgs_) {
    assert(false);
    std::cerr << __FUNCTION__ << ":" << __LINE__ << " ASSERT\n";
    std::cout << "receipt.u0_x0_lgs: " << receipt.u0_x0_lgs << "\n";
    std::cout << "u0_x0_lgs_: " << u0_x0_lgs_ << "\n";

    return false;
  }
  if (receipt.u0d != u0d_) {
    assert(false);
    std::cerr << __FUNCTION__ << ":" << __LINE__ << " ASSERT\n";
    return false;
  }

#ifdef _DEBUG
  auto const& ecc_pub = GetEccPub();
  assert(ecc_pub.PowerU1(0, x0_lgs_) == u0_x0_lgs_);
#endif

  secret.x0_lgs = x0_lgs_;
  secret.d = d_;

#ifdef _DEBUG
  secret.k = k_;
  secret.x = x_;
  secret.m.resize(phantoms_count_ * s_);
  for (size_t i = 0; i < mappings_.size(); ++i) {
    auto const& map = mappings_[i];
    for (uint64_t j = 0; j < s_; ++j) {
      auto golbal_ij = map.global_index * s_ + j;
      secret.m[i * s_ + j] = a_->m()[golbal_ij];
    }
  }
#endif
  return true;
}

template <typename A>
void Session<A>::BuildK() {
  Tick _tick_(__FUNCTION__);
  k_.resize(log_c_ + 1);

  for (size_t p = 0; p < k_.size(); ++p) {
    auto& kp = k_[p];
    size_t rows = align_c_ / (1ULL << p);
    size_t cols = align_s_;
    kp.resize(rows * cols);
    //#pragma omp parallel for (system random generator has internal lock)
    for (size_t i = 0; i < kp.size(); ++i) {
      kp[i] = FrRand();
    }
  }
}

template <typename A>
void Session<A>::BuildX() {
  Tick _tick_(__FUNCTION__);
  assert(k_.size() == (log_c_ + 1));
  x_.resize(log_s_ + 1);

  // x_0 == k_[log_c]
  assert(k_[log_c_].size() == align_s_);
  x_[0] = k_[log_c_];

  for (size_t p = 1; p < x_.size(); ++p) {
    auto& xp = x_[p];
    size_t cols = align_s_ / (1ULL << p);
    xp.resize(cols);
    for (auto& i : xp) {
      i = FrRand();
    }
  }
}

template <typename A>
Fr const& Session<A>::GetK(uint64_t i, uint64_t j, uint64_t p) {
  auto const& kp = k_[p];
  return kp[i * align_s_ + j];
}

template <typename A>
Fr const& Session<A>::GetX(uint64_t j, uint64_t p) {
  auto const& xp = x_[p];
  return xp[j];
}

template <typename A>
void Session<A>::BuildUK(std::vector<std::vector<G1>>& uk) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  uk.resize(log_c_ + 1);

  for (uint64_t p = 0; p < uk.size(); ++p) {
    auto& ukp = uk[p];
    size_t rows = align_c_ / (1ULL << p);
    ukp.resize(rows);
  }

  std::vector<G1> all_g((align_c_ * 2 - 1) * align_s_);
  std::vector<Fr const*> all_f;
  all_f.reserve(all_g.size());
  for (uint64_t p = 0; p < uk.size(); ++p) {
    auto& ukp = uk[p];
    for (uint64_t i = 0; i < ukp.size(); ++i) {
      for (uint64_t j = 0; j < align_s_; ++j) {
        all_f.resize(all_f.size() + 1);
        all_f.back() = &GetK(i, j, p);
      }
    }
  }
  assert(all_f.size() == all_g.size());

#pragma omp parallel for
  for (uint64_t i = 0; i < all_f.size(); ++i) {
    all_g[i] = ecc_pub.PowerU1(i % align_s_, *all_f[i]);
  }

  std::vector<G1*> result;
  result.reserve(align_c_ * 2 - 1);
  for (uint64_t p = 0; p < uk.size(); ++p) {
    auto& ukp = uk[p];
    for (uint64_t i = 0; i < ukp.size(); ++i) {
      result.resize(result.size() + 1);
      result.back() = &ukp[i];
    }
  }
  assert(result.size() == align_c_ * 2 - 1);

#pragma omp parallel for
  for (uint64_t i = 0; i < result.size(); ++i) {
    auto begin = all_g.begin() + align_s_ * i;
    auto end = begin + align_s_;
    *result[i] = std::accumulate(begin, end, G1Zero());
    result[i]->normalize();
  }
}

template <typename A>
void Session<A>::BuildUX0(std::vector<G1>& ux0) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  ux0.resize(align_s_);

#pragma omp parallel for
  for (uint64_t j = 0; j < ux0.size(); ++j) {
    ux0[j] = ecc_pub.PowerU1(j, GetK(0, j, k_.size() - 1));
    ux0[j].normalize();
  }
}

template <typename A>
void Session<A>::BuildU0X(std::vector<std::vector<G1>>& u0x) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  u0x.resize(log_s_ + 1);

  struct Item {
    Fr const* f;
    G1* g;
  };

  std::vector<Item> items;
  items.reserve(align_s_ * 2 - 1);
  for (uint64_t p = 0; p < u0x.size(); ++p) {
    auto& u0xp = u0x[p];
    u0xp.resize(align_s_ / (1ULL << p));
    for (uint64_t j = 0; j < u0xp.size(); ++j) {
      items.resize(items.size() + 1);
      auto& item = items.back();
      item.f = &GetX(j, p);
      item.g = &u0xp[j];
    }
  }

#pragma omp parallel for
  for (uint64_t i = 0; i < items.size(); ++i) {
    *items[i].g = ecc_pub.PowerU1(0, *items[i].f);
    items[i].g->normalize();
  }

  u0_x0_lgs_ = u0x.back()[0];
  x0_lgs_ = *items.back().f;
  assert(x0_lgs_ == GetX(0, log_s_));
}

template <typename A>
void Session<A>::BuildG2X0(std::vector<G2>& g2x0) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  g2x0.resize(align_s_);
#pragma omp parallel for
  for (uint64_t j = 0; j < g2x0.size(); ++j) {
    auto const& x = GetX(j, 0);
    g2x0[j] = ecc_pub.PowerG2(x);
    g2x0[j].normalize();
  }
}

template <typename A>
void Session<A>::BuildOt(std::vector<G1>& ot_ui) {
  ot_rand_c_ = FrRand();
  ot_ui.resize(request_.ot_vi.size());

#pragma omp parallel for
  for (size_t j = 0; j < ot_ui.size(); ++j) {
    ot_ui[j] = request_.ot_vi[j] * ot_rand_c_;
  }
}

template <typename A>
void Session<A>::BlendM(std::vector<Fr>& encrypted_m) {
  Tick _tick_(__FUNCTION__);
#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    Fr fr_e = GetOtFrE(mappings_[i].global_index);
    for (uint64_t j = 0; j < align_s_; ++j) {
      encrypted_m[i * align_s_ + j] += fr_e;
    }
  }
}

template <typename A>
Fr Session<A>::GetOtFrE(uint64_t global_index) {
  auto fr_i = MapToFr(global_index);
  Fp12 e;
  G1 v_exp_fr_c = request_.ot_v * (fr_i * ot_rand_c_);
  mcl::bn256::pairing(e, v_exp_fr_c, ot_sk_);
  uint8_t buf[32 * 12];
  auto ret_len = e.serialize(buf, sizeof(buf));
  if (ret_len != sizeof(buf)) {
    assert(false);
    throw std::runtime_error("oops");
  }
  return MapToFr(buf, sizeof(buf));
}

template <typename A>
void Session<A>::BuildM(std::vector<Fr>& encrypted_m) {
  Tick _tick_(__FUNCTION__);
  // mij' = kij0 + d + c^i * mij
  auto const& m = a_->m();
  encrypted_m.resize(phantoms_count_ * align_s_);
  Fr zero = FrZero();

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto const& map = mappings_[i];
    for (uint64_t j = 0; j < align_s_; ++j) {
      Fr const& mij = j < s_ ? m[map.global_index * s_ + j] : zero;
      encrypted_m[i * align_s_ + j] = GetK(i, j, 0) + d_ + challenge_.c * mij;
    }
  }
}

template <typename A>
void Session<A>::BuildEK(std::vector<std::vector<Fr>>& ek) {
  Tick _tick_(__FUNCTION__);
  auto f1 = [this](uint64_t i, uint64_t j, uint64_t p) {
    auto const& a = GetK(i, j, p + 1);
    auto const& b = GetK(2 * i, j, p);
    auto const& c = GetK(2 * i + 1, j, p);
    return a + challenge_.e1 * b + challenge_.e1_square * c;
  };
  auto f2 = [this](uint64_t i, uint64_t j, uint64_t p) {
    auto const& a = GetK(i, j, p + 1);
    auto const& b = GetK(2 * i, j, p);
    auto const& c = GetK(2 * i + 1, j, p);
    return a + challenge_.e2 * b + challenge_.e2_square * c;
  };

  ek.resize(log_c_);
  for (size_t p = 0; p < log_c_; ++p) {
    auto& kep = ek[p];
    size_t rows = align_c_ / (1ULL << p);
    size_t cols = align_s_;
    kep.resize(rows * cols);
    for (size_t i = 0; i < rows / 2; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        size_t row = 2 * i;
        size_t col = j;
        kep[row * cols + col] = f1(i, j, p);
        ++row;
        kep[row * cols + col] = f2(i, j, p);
      }
    }
  }
}

template <typename A>
void Session<A>::BuildEX(std::vector<std::vector<Fr>>& ex) {
  Tick _tick_(__FUNCTION__);
  auto f1 = [this](uint64_t j, uint64_t p) {
    auto const& a = GetX(j, p + 1);
    auto const& b = GetX(2 * j, p);
    auto const& c = GetX(2 * j + 1, p);
    return a + challenge_.e1 * b + challenge_.e1_square * c;
  };
  auto f2 = [this](uint64_t j, uint64_t p) {
    auto const& a = GetX(j, p + 1);
    auto const& b = GetX(2 * j, p);
    auto const& c = GetX(2 * j + 1, p);
    return a + challenge_.e2 * b + challenge_.e2_square * c;
  };

  ex.resize(log_s_);
  for (size_t p = 0; p < log_s_; ++p) {
    auto& exp = ex[p];
    size_t cols = align_s_ / (1ULL << (p + 1));
    exp.resize(cols * 2);  // rows = 2
    for (size_t j = 0; j < cols; ++j) {
      exp[j] = f1(j, p);
      exp[cols + j] = f2(j, p);
    }
  }
}

template <typename A>
void Session<A>::BuildCommitmentD(std::vector<G1>& ud, G2& g2d) {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  g2d = ecc_pub.PowerG2(d_);
  ud.resize(align_s_);

#pragma omp parallel for
  for (size_t j = 0; j < align_s_; ++j) {
    ud[j] = ecc_pub.PowerU1(j, d_);
    ud[j].normalize();
  }

  u0d_ = ud[0];
}

}  // namespace scheme::otbatch3
