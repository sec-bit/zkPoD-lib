#pragma once

#include <stdint.h>
#include <vector>

#include "ecc.h"
#include "scheme_atomic_swap_vc_zkp.h"

namespace scheme::atomic_swap_vc {

inline uint64_t PadCountOfCol(uint64_t n) {
  uint64_t kZkFrCount = ZkpMimcCount() - 1;
  return (n + kZkFrCount - 1) / kZkFrCount;
}

struct ZkItem {
  std::vector<uint64_t> public_offset;  // MimcInv(seed0+offset)
  std::vector<Fr> public_w;
  std::vector<uint64_t> col;
  uint64_t row;
};

void BuildZkItems(std::vector<ZkItem>& zk_items, uint64_t n, uint64_t s,
                  std::vector<Fr> const& w);

void BuildZkIpVw(std::vector<Fr>& zk_ip_vw, std::vector<Fr> const& v,
                 std::vector<ZkItem> const& zk_items);

void BuildZkProofs(std::vector<ZkProof>& zk_proofs, ZkPk const& pk,
                   std::vector<ZkItem> const& zk_items,
                   std::vector<Fr> const& zk_ip_vw, Fr const& seed,
                   Fr const& seed_rand, Fr seed_mimc3_digest,
                   ZkVkPtr check_vk);

bool VerifyZkProofs(std::vector<ZkProof> const& zk_proofs, ZkVk const& vk,
                    std::vector<ZkItem> const& zk_items,
                    std::vector<Fr> const& zk_ip_vw,
                    Fr const& seed_mimc3_digest);
}  // namespace scheme::atomic_swap_vc
