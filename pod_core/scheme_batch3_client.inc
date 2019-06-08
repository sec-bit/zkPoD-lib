
namespace scheme::batch3 {

template <typename B>
Client<B>::Client(std::shared_ptr<B> b, h256_t const& self_id,
                  h256_t const& peer_id, std::vector<Range> demands)
    : b_(b),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(b_->bulletin().n),
      s_(b_->bulletin().s),
      demands_(std::move(demands)) {
  if (!CheckDemands(n_, demands_)) {
    throw std::invalid_argument("invalid demands");
  }

  for (auto const& i : demands_) {
    demands_count_ += i.count;
  }

  BuildMapping();
  align_c_ = misc::Pow2UB(demands_count_);
  align_s_ = misc::Pow2UB(s_);
  log_c_ = misc::Log2UB(align_c_);
  log_s_ = misc::Log2UB(align_s_);
}

template <typename B>
void Client<B>::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(demands_count_);
  size_t index = 0;
  for (auto const& d : demands_) {
    for (size_t i = d.start; i < (d.start + d.count); ++i) {
      auto& map = mappings_[index];
      map.global_index = i;
      ++index;
    }
  }
}

template <typename B>
void Client<B>::GetRequest(Request& request) {
  request.demands = demands_;
  request_ = request;
}

template <typename B>
bool Client<B>::OnResponse(Response response, Receipt& receipt) {
  Tick _tick_(__FUNCTION__);
  response_ = std::move(response);

  // check commitment data format
  if (response_.uk.size() != (log_c_ + 1)) {
    assert(false);
    return false;
  }

  for (uint64_t p = 0; p < response_.uk.size(); ++p) {
    auto& ukp = response_.uk[p];
    auto rows = align_c_ / (1LL << p);
    if (ukp.size() != rows) {
      assert(false);
      return false;
    }
  }

  if (response_.ux0.size() != align_s_) {
    assert(false);
    return false;
  }

  if (response_.u0x.size() != (log_s_ + 1)) {
    assert(false);
    return false;
  }

  for (uint64_t p = 0; p < response_.u0x.size(); ++p) {
    auto& u0xp = response_.u0x[p];
    auto cols = align_s_ / (1LL << p);
    if (u0xp.size() != cols) {
      assert(false);
      return false;
    }
  }

  if (response_.g2x0.size() != align_s_) {
    assert(false);
    return false;
  }

  if (response_.ud.size() != align_s_) {
    assert(false);
    return false;
  }

  // check commitment of d
  if (!CheckCommitmentOfD()) {
    assert(false);
    return false;
  }

  // rom the challenge the seed
  scheme::batch3::ComputeChallenge(challenge_, self_id_, peer_id_, b_->bulletin(), request_,
                   response_);

  // check encrypted data format
  if (response_.m.size() != demands_count_ * align_s_) {
    assert(false);
    return false;
  }

  // check encrypted ek, ex format
  if (response_.ek.size() != log_c_) {
    assert(false);
    return false;
  }

  for (size_t p = 0; p < response_.ek.size(); ++p) {
    auto const& ekp = response_.ek[p];
    auto rows = align_c_ / (1LL << p);
    auto cols = align_s_;
    if (ekp.size() != rows * cols) {
      assert(false);
      return false;
    }
  }

  if (response_.ex.size() != log_s_) {
    assert(false);
    return false;
  }

  for (size_t p = 0; p < response_.ex.size(); ++p) {
    auto const& exp = response_.ex[p];
    auto cols = align_s_ / (1LL << (p + 1));
    if (exp.size() != 2 * cols) {
      assert(false);
      return false;
    }
  }

  encrypted_m_ = std::move(response_.m);

  // check valid
  if (!CheckEncryptedM()) {
    assert(false);
    return false;
  }

  if (!CheckUX0()) {
    assert(false);
    return false;
  }

  if (!CheckEK()) {
    assert(false);
    return false;
  }

  if (!CheckEX()) {
    assert(false);
    return false;
  }

  receipt.u0d = response_.ud[0];
  receipt.u0_x0_lgs = response_.u0x.back()[0];

  receipt_ = receipt;
  return true;
}

template <typename B>
bool Client<B>::CheckEncryptedM() {
  Tick _tick_(__FUNCTION__);

  auto const& ecc_pub = GetEccPub();
  auto const& sigmas = b_->sigmas();

  auto const& ud = response_.ud;
  assert(ud.size() == align_s_);

  auto const& uk0 = response_.uk[0];
  assert(uk0.size() == align_c_);

  std::vector<Fr> w(demands_count_);
  for (auto& i : w) i = FrRand();

  // left
  std::vector<G1> left_g(demands_count_ * 2);
  for (size_t i = 0; i < demands_count_; ++i) {
    auto const& mapping = mappings_[i];
    G1 const& sigma = sigmas[mapping.global_index];
    left_g[i] = sigma;
    left_g[i + demands_count_] = uk0[i];
  }
  std::vector<Fr> left_f(left_g.size());
  for (size_t i = 0; i < demands_count_; ++i) {
    left_f[i] = challenge_.c * w[i];
    left_f[i + demands_count_] = w[i];
  }
  G1 sigma_ud = std::accumulate(ud.begin(), ud.end(), G1Zero());
  G1 sigma_udn = sigma_ud * std::accumulate(w.begin(), w.end(), FrZero());
  G1 left = MultiExpBdlo12(left_g, left_f) + sigma_udn;

  // right
  std::vector<Fr> right_f(align_s_);
  for (size_t j = 0; j < right_f.size(); ++j) {
    right_f[j] = FrZero();
    for (size_t i = 0; i < demands_count_; ++i) {
      right_f[j] += w[i] * encrypted_m_[i * align_s_ + j];
    }
  }
  std::vector<G1> right_g(align_s_);
#pragma omp parallel for
  for (size_t j = 0; j < right_f.size(); ++j) {
    right_g[j] = ecc_pub.PowerU1(j, right_f[j]);
  }
  G1 right = std::accumulate(right_g.begin(), right_g.end(), G1Zero());

  if (left != right) {
    std::cout << __FUNCTION__ << " " << __LINE__ << " ASSERT\n";
    assert(false);
    return false;
  }
  return true;
}

template <typename B>
bool Client<B>::OnSecret(Secret secret) {
  Tick _tick_(__FUNCTION__);

  secret_ = std::move(secret);

  if (!VerifyProof(receipt_, secret_)) {
    assert(false);
    return false;
  }

  DecryptX();
  DecryptK();
  DecryptM();

  return true;
}

template <typename B>
void Client<B>::DecryptM() {
  Tick _tick_(__FUNCTION__);

  Fr inv_c = FrInv(challenge_.c);

  decrypted_m_.resize(demands_count_ * s_);

  auto const& k0 = k_[0];
#pragma omp parallel for
  for (size_t i = 0; i < mappings_.size(); ++i) {
    for (uint64_t j = 0; j < s_; ++j) {
      auto index1 = i * s_ + j;
      auto index2 = i * align_s_ + j;
      decrypted_m_[index1] =
          (encrypted_m_[index2] - k0[index2] - secret_.d) * inv_c;
    }
  }

#ifdef _DEBUG
  if (!secret_.m.empty()) {
    assert(decrypted_m_ == secret_.m);
  }
#endif
}

template <typename B>
bool Client<B>::SaveDecrypted(std::string const& file) {
  Tick _tick_(__FUNCTION__);
  return b_->SaveDecryped(file, demands_, decrypted_m_);
}

template <typename B>
bool Client<B>::CheckCommitmentOfD() {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();

  int failed = 0;
#pragma omp parallel for
  for (size_t j = 0; j < align_s_; ++j) {
    if (failed) continue;
    auto const& ujd = response_.ud[j];
    auto const& uj = ecc_pub.u1()[j];
    auto const& g2d = response_.g2d;
    if (!PairingMatch(ujd, uj, g2d)) {
      std::cout << __FUNCTION__ << " " << __LINE__ << " ASSERT\n";
#pragma omp atomic
      ++failed;
      assert(false);
    }
  }
  return failed == 0;
}

template <typename B>
bool Client<B>::CheckUX0() {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  auto const& ux0 = response_.ux0;
  auto const& u0x0 = response_.u0x[0];
  auto const& g2x0 = response_.g2x0;

  int failed = 0;
#pragma omp parallel for
  for (size_t j = 0; j < align_s_; ++j) {
    if (failed) continue;
    auto const& uj_xj_0 = ux0[j];
    auto const& uj = ecc_pub.u1()[j];
    auto const& u0 = ecc_pub.u1()[0];
    auto const& g2_xj_0 = g2x0[j];
    auto const& u0_xj_0 = u0x0[j];
    if (!PairingMatch(uj_xj_0, uj, g2_xj_0)) {
      assert(false);
#pragma omp atomic
      ++failed;
      continue;
    }
    if (!PairingMatch(u0_xj_0, u0, g2_xj_0)) {
      assert(false);
#pragma omp atomic
      ++failed;
      continue;
    }
  }
  return failed == 0;
}

template <typename B>
bool Client<B>::CheckEX() {
  Tick _tick_(__FUNCTION__);
  auto const& ecc_pub = GetEccPub();
  auto const& ex = response_.ex;
  auto const& u0x = response_.u0x;

  struct Item {
    Fr const* ex1;
    Fr const* ex2;
    G1 const* ux1;
    G1 const* ux2;
    G1 const* ux3;
  };

  std::vector<Item> items;
  items.reserve(align_s_ - 1);

  for (size_t p = 0; p < log_s_; ++p) {
    auto const& exp = ex[p];
    auto const& u0xp = u0x[p];
    auto const& u0xp_1 = u0x[p + 1];
    auto cols = align_s_ / (1ULL << (p + 1));
    assert(exp.size() == cols * 2);

    for (size_t j = 0; j < cols; ++j) {
      items.resize(items.size() + 1);
      auto& item = items.back();
      item.ex1 = &exp[j];
      item.ex2 = &exp[cols + j];
      item.ux1 = &u0xp_1[j];
      item.ux2 = &u0xp[2 * j];
      item.ux3 = &u0xp[2 * j + 1];
    }
  }

  assert(items.size() == align_s_ - 1);
  WN wn(challenge_, items.size());

  Fr left_f = FrZero();
  for (size_t j = 0; j < items.size(); ++j) {
    auto const& w = wn.w(j);
    auto const& item = items[j];
    left_f += w.w1 * (*item.ex1) + w.w2 * (*item.ex2);
  }
  G1 left = ecc_pub.PowerU1(0, left_f);

  std::vector<Fr const*> right_f(items.size() * 3);
  std::vector<G1 const*> right_g(right_f.size());
  for (size_t j = 0; j < items.size(); ++j) {
    auto const& w = wn.w(j);
    auto const& item = items[j];
    right_f[3 * j] = &w.w1_w2;
    right_f[3 * j + 1] = &w.w1e1_w2e2;
    right_f[3 * j + 2] = &w.w1e1e1_w2e2e2;
    right_g[3 * j] = item.ux1;
    right_g[3 * j + 1] = item.ux2;
    right_g[3 * j + 2] = item.ux3;
  }

  G1 right = MultiExpBdlo12(right_g, right_f);

  if (left != right) {
    std::cerr << __FUNCTION__ << ":" << __LINE__ << " ASSERT\n";
    assert(false);
    return false;
  }

  return true;
}

template <typename B>
bool Client<B>::CheckEK() {
  Tick _tick_(__FUNCTION__);
  if (align_c_ == 1) return true;

  auto const& ecc_pub = GetEccPub();
  auto const& ek = response_.ek;
  auto const& uk = response_.uk;

  struct Item {
    std::vector<Fr const*> ek1;
    std::vector<Fr const*> ek2;
    G1 const* uk1;
    G1 const* uk2;
    G1 const* uk3;
  };

  std::vector<Item> items;
  items.reserve(align_c_ - 1);
  for (size_t p = 0; p < log_c_; ++p) {
    auto const& ukp = uk[p];
    auto const& ekp = ek[p];
    auto rows = align_c_ / (1ULL << p);
    auto cols = align_s_;
    assert(ekp.size() == rows * cols);
    for (size_t i = 0; i < rows / 2; ++i) {
      items.resize(items.size() + 1);
      auto& item = items.back();
      item.ek1.resize(align_s_);
      item.ek2.resize(align_s_);
      for (size_t j = 0; j < align_s_; ++j) {
        item.ek1[j] = &ekp[2 * i * cols + j];
        item.ek2[j] = &ekp[(2 * i + 1) * cols + j];
      }

      auto const& ukp_1 = uk[p + 1];
      item.uk1 = &ukp_1[i];
      item.uk2 = &ukp[2 * i];
      item.uk3 = &ukp[2 * i + 1];
    }
  }
  assert(items.size() == align_c_ - 1);
  WN wn(challenge_, items.size());

  std::vector<Fr> left_f(align_s_);
  for (size_t j = 0; j < align_s_; ++j) {
    left_f[j] = FrZero();
    for (size_t i = 0; i < items.size(); ++i) {
      auto const& item = items[i];
      left_f[j] += wn.w(i).w1 * (*item.ek1[j]);
      left_f[j] += wn.w(i).w2 * (*item.ek2[j]);
    }
  }
  std::vector<G1> left_g(align_s_);
  for (size_t j = 0; j < align_s_; ++j) {
    left_g[j] = ecc_pub.PowerU1(j, left_f[j]);
  }
  G1 left = std::accumulate(left_g.begin(), left_g.end(), G1Zero());

  std::vector<Fr const*> right_f(items.size() * 3);
  std::vector<G1 const*> right_g(right_f.size());
  for (size_t i = 0; i < items.size(); ++i) {
    auto const& item = items[i];
    auto const& w = wn.w(i);
    right_f[3 * i] = &w.w1_w2;
    right_f[3 * i + 1] = &w.w1e1_w2e2;
    right_f[3 * i + 2] = &w.w1e1e1_w2e2e2;
    right_g[3 * i] = item.uk1;
    right_g[3 * i + 1] = item.uk2;
    right_g[3 * i + 2] = item.uk3;
  }
  G1 right = MultiExpBdlo12(right_g, right_f);

  if (left != right) {
    std::cerr << __FUNCTION__ << ":" << __LINE__ << " ASSERT\n";
    assert(false);
    return false;
  }

  return true;
}

template <typename B>
void Client<B>::DecryptK() {
  Tick _tick_(__FUNCTION__);
  k_.resize(log_c_ + 1);
  for (size_t p = 0; p < k_.size(); ++p) {
    auto& kp = k_[p];
    size_t rows = align_c_ / (1ULL << p);
    size_t cols = align_s_;
    kp.resize(rows * cols);
  }

  // x_0 == k_[log_c]
  assert(x_[0].size() == k_[log_c_].size());
  k_[log_c_] = x_[0];

  // NOTE: can not parallel
  auto const& ek = response_.ek;
  for (int64_t p = (int64_t)log_c_ - 1; p >= 0; --p) {
    auto& kp = k_[p];
    auto& kp_1 = k_[p + 1];
    auto const& ekp = ek[p];
    auto kp_1_rows = align_c_ / (1ULL << (p + 1));
    auto cols = align_s_;
    for (size_t i = 0; i < kp_1_rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        kp[2 * i * cols + j] =
            challenge_.e2_square * (ekp[2 * i * cols + j] - kp_1[i * cols + j]);
        kp[2 * i * cols + j] -=
            challenge_.e1_square *
            (ekp[(2 * i + 1) * cols + j] - kp_1[i * cols + j]);
        kp[2 * i * cols + j] *= challenge_.e1_e2_inverse;

        kp[(2 * i + 1) * cols + j] =
            challenge_.e1 * (ekp[(2 * i + 1) * cols + j] - kp_1[i * cols + j]);
        kp[(2 * i + 1) * cols + j] -=
            challenge_.e2 * (ekp[2 * i * cols + j] - kp_1[i * cols + j]);
        kp[(2 * i + 1) * cols + j] *= challenge_.e1_e2_inverse;
      }
    }
  }

#ifdef _DEBUG
  if (!secret_.k.empty()) {
    assert(k_ == secret_.k);
  }
#endif
}

template <typename B>
void Client<B>::DecryptX() {
  Tick _tick_(__FUNCTION__);
  x_.resize(log_s_ + 1);
  for (size_t p = 0; p < x_.size(); ++p) {
    auto& xp = x_[p];
    size_t cols = align_s_ / (1ULL << p);
    xp.resize(cols);
  }
  assert(x_[log_s_].size() == 1);
  x_[log_s_][0] = secret_.x0_lgs;

  // NOTE: can not parallel
  auto const& ex = response_.ex;
  for (int64_t p = (int64_t)log_s_ - 1; p >= 0; --p) {
    auto& xp = x_[p];
    auto const& xp_1 = x_[p + 1];
    auto const& exp = ex[p];
    auto exp_cols = align_s_ / (1ULL << (p + 1));
    for (size_t j = 0; j < xp_1.size(); ++j) {
      xp[2 * j] = challenge_.e2_square * (exp[j] - xp_1[j]);
      xp[2 * j] -= challenge_.e1_square * (exp[exp_cols + j] - xp_1[j]);
      xp[2 * j] *= challenge_.e1_e2_inverse;

      xp[2 * j + 1] = challenge_.e1 * (exp[exp_cols + j] - xp_1[j]);
      xp[2 * j + 1] -= challenge_.e2 * (exp[j] - xp_1[j]);
      xp[2 * j + 1] *= challenge_.e1_e2_inverse;
    }
  }

#ifdef _DEBUG
  if (!secret_.x.empty()) {
    assert(x_ == secret_.x);
  }
#endif
}
}  // namespace scheme::batch3
