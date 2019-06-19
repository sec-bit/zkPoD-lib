
namespace scheme::complaint {

template <typename AliceData>
Session<AliceData>::Session(std::shared_ptr<AliceData> a, h256_t const& self_id,
                            h256_t const& peer_id)
    : a_(a),
      self_id_(self_id),
      peer_id_(peer_id),
      n_(a_->bulletin().n),
      s_(a_->bulletin().s) {
  seed0_ = misc::RandH256();
}

template <typename AliceData>
void Session<AliceData>::BuildMapping() {
  Tick _tick_(__FUNCTION__);
  mappings_.resize(demands_count_);
  size_t index = 0;
  for (auto const& p : demands_) {
    for (size_t i = p.start; i < (p.start + p.count); ++i) {
      mappings_[index++].global_index = i;
    }
  }
}

template <typename AliceData>
bool Session<AliceData>::OnRequest(Request request, Response& response) {
  Tick _tick_(__FUNCTION__);

  if (!CheckDemands(n_, request.demands)) {
    assert(false);
    return false;
  }

  for (auto const& i : request.demands) demands_count_ += i.count;

  demands_ = std::move(request.demands);
  seed2_seed_ = request.seed2_seed;

  BuildMapping();

  H2(seed0_, demands_count_ * s_, v_);

  if (evil_) {
    // NOTE: use rand() for test
    uint64_t evil_i = rand() % demands_count_;
    uint64_t evil_j = s_ - 1;  // last col
    v_[evil_i * s_ + evil_j] = FrRand();
    std::cout << "evil: " << evil_i << "," << evil_j << "\n";
  }

  BuildK(v_, response.k, s_);

  k_mkl_root_ = CalcRootOfK(response.k);

  seed2_ = CalcSeed2(seed2_seed_, k_mkl_root_);

  H2(seed2_, demands_count_, w_);

  // compute mij' = vij + wi * mij
  auto const& m = a_->m();
  response.m.resize(demands_count_ * s_);

#pragma omp parallel for
  for (int64_t i = 0; i < (int64_t)mappings_.size(); ++i) {
    auto const& map = mappings_[i];
    auto is = i * s_;
    auto m_is = map.global_index * s_;
    for (uint64_t j = 0; j < s_; ++j) {
      auto ij = is + j;
      auto m_ij = m_is + j;
      response.m[ij] = v_[ij] + w_[i] * m[m_ij];
    }
  }

  return true;
}

template <typename AliceData>
bool Session<AliceData>::OnReceipt(Receipt const& receipt, Secret& secret) {
  if (receipt.seed2 != seed2_) {
    assert(false);
    return false;
  }
  if (receipt.k_mkl_root != k_mkl_root_) {
    assert(false);
    return false;
  }
  if (receipt.count != demands_count_) {
    assert(false);
    return false;
  }
  secret.seed0 = seed0_;
  return true;
}

}  // namespace scheme::complaint
