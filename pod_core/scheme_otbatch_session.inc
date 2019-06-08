
namespace scheme::otbatch {

template <typename A>
Session<A>::Session(std::shared_ptr<A> a, h256_t const& self_id,
                    h256_t const& peer_id)
    : a_(a),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(a_->bulletin().n),
      s_(a_->bulletin().s) {
  seed0_ = misc::RandH256();
  ot_self_pk_ = G2Rand();
  ot_alpha_ = FrRand();
}

template <typename A>
void Session<A>::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(phantoms_count_);
  size_t index = 0;
  for (auto const& p : phantoms_) {
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

  phantoms_ = request.phantoms;       // sizeof() = L
  ot_vi_ = std::move(request.ot_vi);  // sizeof() = K
  ot_v_ = std::move(request.ot_v);
  seed2_seed_ = request.seed2_seed;

  if (!CheckPhantoms(n_, phantoms_)) {
    assert(false);
    return false;
  }

  for (auto const& i : phantoms_) phantoms_count_ += i.count;

  if (ot_vi_.size() >= phantoms_count_) {
    assert(false);
    return false;
  }

  BuildMapping();

  H2(seed0_, phantoms_count_ * s_, v_);

  if (evil_) {
    // NOTE: use rand() for test
    uint64_t evil_i = rand() % phantoms_count_;
    uint64_t evil_j = s_ - 1;  // last col
    v_[evil_i * s_ + evil_j] = FrRand();
    std::cout << "evil: " << evil_i << "," << evil_j << "\n";
  }

  BuildK(v_, response.k, s_);

  k_mkl_root_ = CalcRootOfK(response.k);

  ot_rand_c_ = FrRand();
  response.ot_ui.resize(ot_vi_.size());

#pragma omp parallel for
  for (int64_t j = 0; j < (int64_t)response.ot_ui.size(); ++j) {
    response.ot_ui[j] = ot_vi_[j] * ot_rand_c_;
  }

  seed2_ = CalcSeed2(seed2_seed_, k_mkl_root_);

  H2(seed2_, phantoms_count_, w_);

  // compute mij' = vij + wi * mij
  auto const& m = a_->m();
  response.m.resize(phantoms_count_ * s_);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto const& map = mappings_[i];
    auto fr_i = MapToFr(map.global_index);
    Fp12 e;
    G1 v_exp_fr_c = ot_v_ * (fr_i * ot_rand_c_);
    mcl::bn256::pairing(e, v_exp_fr_c, ot_sk_);
    uint8_t buf[32 * 12];
    auto ret_len = e.serialize(buf, sizeof(buf));
    if (ret_len != sizeof(buf)) {
      assert(false);
      throw std::runtime_error("oops");
    }
    Fr fr_e = MapToFr(buf, sizeof(buf));

    auto is = i * s_;
    auto m_is = map.global_index * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      auto m_ij = m_is + j;
      response.m[ij] = v_[ij] + w_[i] * m[m_ij];
      response.m[ij] += fr_e;
    }
  }

  return true;
}

template <typename A>
bool Session<A>::OnReceipt(Receipt const& receipt, Secret& secret) {
  if (receipt.seed2 != seed2_) {
    assert(false);
    return false;
  }
  if (receipt.k_mkl_root != k_mkl_root_) {
    assert(false);
    return false;
  }
  if (receipt.count != phantoms_count_) {
    assert(false);
    return false;
  }
  secret.seed0 = seed0_;
  return true;
}

}  // namespace scheme::otbatch
