#include "scheme_atomic_swap_vc_misc.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <numeric>
#include "scheme_atomic_swap_vc_zkp.h"
#include "tick.h"

namespace scheme::atomic_swap_vc {

namespace {
void ConvertToZkpItems(std::vector<ZkpItem>& zkp_items,
                       std::vector<ZkItem> const& zk_items,
                       std::vector<Fr> const& zk_ip_vw, Fr const& seed,
                       Fr const& seed_rand, Fr seed_mimc3_digest) {
  assert(zk_items.size() == zk_ip_vw.size());
  zkp_items.resize(zk_items.size());
  for (size_t i = 0; i < zk_items.size(); ++i) {
    zkp_items[i].o = ConvertToZkFr(zk_items[i].public_offset);
    zkp_items[i].w = ConvertToZkFr(zk_items[i].public_w);
    zkp_items[i].seed = ConvertToZkFr(seed);
    zkp_items[i].seed_rand = ConvertToZkFr(seed_rand);
    zkp_items[i].seed_mimc3_digest = ConvertToZkFr(seed_mimc3_digest);
    zkp_items[i].inner_product = ConvertToZkFr(zk_ip_vw[i]);
  }
}

void ConvertToZkvItems(std::vector<ZkvItem>& zkv_items,
                       std::vector<ZkItem> const& zk_items,
                       std::vector<Fr> const& zk_ip_vw, Fr seed_mimc3_digest) {
  assert(zk_items.size() == zk_ip_vw.size());
  zkv_items.resize(zk_items.size());
  for (size_t i = 0; i < zk_items.size(); ++i) {
    zkv_items[i].o = ConvertToZkFr(zk_items[i].public_offset);
    zkv_items[i].w = ConvertToZkFr(zk_items[i].public_w);
    zkv_items[i].seed_mimc3_digest = ConvertToZkFr(seed_mimc3_digest);
    zkv_items[i].inner_product = ConvertToZkFr(zk_ip_vw[i]);
  }
}
}  // namespace

void BuildZkItems(std::vector<ZkItem>& zk_items, uint64_t n, uint64_t s,
                  std::vector<Fr> const& w) {
  assert(w.size() == n);
  auto const kMimcCount = ZkpMimcCount();
  auto pad_count_of_col = PadCountOfCol(n);
  auto nn = n + pad_count_of_col;
  if (nn <= kMimcCount) {
    assert(pad_count_of_col == 1);
    // one proof contains multiple col
    auto col_count = kMimcCount / nn;
    zk_items.resize((s + col_count - 1) / col_count);
    for (size_t i = 0; i < zk_items.size(); ++i) {
      auto& item = zk_items[i];
      item.row = 0;
      for (size_t j = 0; j < col_count; ++j) {
        auto col = j + i * col_count;
        if (col >= s) continue;
        item.col.push_back(col);
      }
    }
  } else {
    // one proof contains kMimicCount (part of col)
    zk_items.resize(pad_count_of_col * s);
    for (uint64_t j = 0; j < s; ++j) {
      for (uint64_t i = 0; i < pad_count_of_col; ++i) {
        auto& item = zk_items[pad_count_of_col * j + i];
        item.row = i;
        item.col.push_back(j);  // only one column
      }
    }
  }

  for (auto& item : zk_items) {
    item.public_offset.reserve(kMimcCount);
    item.public_w.reserve(kMimcCount);
    for (auto j : item.col) {
      // add spring
      auto offset = (n + item.row) * s + j;
      item.public_offset.push_back(offset);
      item.public_w.push_back(Fr(1));

      // add element
      auto start_i = (kMimcCount - 1) * item.row;
      auto end_i = (kMimcCount - 1) * (item.row + 1);
      if (end_i > n) end_i = n;
      for (auto i = start_i; i < end_i; ++i) {
        offset = i * s + j;
        item.public_offset.push_back(offset);
        item.public_w.push_back(w[i]);
      }
    }

    auto pad_count = kMimcCount - item.public_w.size();
    for (uint64_t i = 0; i < pad_count; ++i) {
      item.public_offset.push_back(0);
      item.public_w.push_back(Fr(0));
    }
    assert(item.public_offset.size() == kMimcCount);
    assert(item.public_w.size() == kMimcCount);
  }
}

void BuildZkIpVw(std::vector<Fr>& zk_ip_vw, std::vector<Fr> const& v,
                 std::vector<ZkItem> const& zk_items) {
  zk_ip_vw.resize(zk_items.size());
  for (size_t i = 0; i < zk_ip_vw.size(); ++i) {
    auto const& item = zk_items[i];
    auto& ip_vw = zk_ip_vw[i];
    ip_vw = FrZero();
    for (size_t j = 0; j < item.public_w.size(); ++j) {
      auto offset = item.public_offset[j];
      ip_vw += v[offset] * item.public_w[j];
    }
  }
}

void BuildZkProofs(std::vector<ZkProof>& zk_proofs, ZkPk const& pk,
                   std::vector<ZkItem> const& zk_items,
                   std::vector<Fr> const& zk_ip_vw, Fr const& seed,
                   Fr const& seed_rand, Fr seed_mimc3_digest,
                   ZkVkPtr check_vk) {
  Tick tick(__FUNCTION__);
  std::vector<ZkpItem> zkp_items;
  ConvertToZkpItems(zkp_items, zk_items, zk_ip_vw, seed, seed_rand,
                    seed_mimc3_digest);

  zk_proofs.resize(zk_items.size());
  std::cout << "BuildZkProofs: zk_items.size() " << zk_items.size()
            << "\n";

#ifdef MULTICORE
#ifndef _DEBUG
#pragma omp parallel for
#endif
#endif
  for (size_t i = 0; i < zk_items.size(); ++i) {
    GenerateZkProof(zk_proofs[i], pk, zkp_items[i], check_vk);
  }
}

bool VerifyZkProofs(std::vector<ZkProof> const& zk_proofs, ZkVk const& vk,
                    std::vector<ZkItem> const& zk_items,
                    std::vector<Fr> const& zk_ip_vw,
                    Fr const& seed_mimc3_digest) {
  Tick tick(__FUNCTION__);
  assert(zk_items.size() == zk_ip_vw.size());
  if (zk_proofs.size() != zk_items.size()) {
    assert(false);
    return false;
  }

  std::vector<ZkvItem> zkv_items;
  ConvertToZkvItems(zkv_items, zk_items, zk_ip_vw, seed_mimc3_digest);

  for (size_t i = 0; i < zk_items.size(); ++i) {
    if (!VerifyZkProof(zk_proofs[i], vk, zkv_items[i])) return false;
  }
  return true;
}
}  // namespace scheme::atomic_swap_vc
