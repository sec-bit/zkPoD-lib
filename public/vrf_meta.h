#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "basic_types.h"
#include "bp.h"
#include "vrf.h"

namespace scheme::table {
struct Bulletin;

struct VrfKeyMeta {
  uint64_t column_index;
  uint64_t j;
  h256_t mj_mkl_root;
  h256_t bp_digest;
  bool unique;
  bool operator==(VrfKeyMeta const& v) const;
  bool operator!=(VrfKeyMeta const& v) const;
};

struct VrfMeta {
  h256_t pk_digest;
  std::vector<std::string> column_names;
  std::vector<VrfKeyMeta> keys;
  bool valid() const;
  bool operator==(VrfMeta const& v) const;
  bool operator!=(VrfMeta const& v) const;
};

bool SaveVrfMeta(std::string const& output, VrfMeta const& data);

bool LoadVrfMeta(std::string const& input, h256_t const* digest, VrfMeta& data);

bool SaveVrfPk(std::string const& output, vrf::Pk<> const& pk);

bool LoadVrfPk(std::string const& file, h256_t const* digest, vrf::Pk<>& pk);

bool SaveVrfSk(std::string const& output, vrf::Sk<> const& sk);

bool LoadVrfSk(std::string const& file, vrf::Sk<>& sk);

bool SaveBpP1Proof(std::string const& output, bp::P1Proof const& proof);

bool LoadBpP1Proof(std::string const& file, h256_t const& digest,
                   bp::P1Proof& proof);

void BuildKeyBp(uint64_t n, uint64_t s, std::vector<Fr> const& m,
                h256_t const& sigma_mkl_root, uint64_t key_pos,
                h256_t keycol_mkl_root, bp::P1Proof& p1_proof);

bool VerifyKeyBp(uint64_t n, uint64_t s, std::vector<Fr> const& km,
                 std::vector<G1> const& sigmas, uint64_t key_pos,
                 h256_t const& sigma_mkl_root, h256_t keycol_mkl_root,
                 bp::P1Proof const& p1_proof);

bool VerifyKeyBp(std::string const& file, Bulletin const& bulletin,
                 VrfMeta const& vrf_meta, uint64_t j, std::vector<Fr> const& km,
                 std::vector<G1> const& sigmas);

h256_t HashVrfKey(std::string const& k, vrf::Sk<> const& vrf_sk);

bool LoadKeyM(std::string const& input, uint64_t n, bool unique,
              h256_t const* root, std::vector<Fr>& km);
}  // namespace scheme::table