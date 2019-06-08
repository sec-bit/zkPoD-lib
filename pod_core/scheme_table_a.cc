#include "scheme_table_a.h"
#include "public.h"
#include "misc.h"

namespace {

}  // namespace

namespace scheme::table {

A::A(std::string const& publish_path) : publish_path_(publish_path) {
  std::string public_path = publish_path_ +"/public";
  std::string private_path = publish_path_ + "/private";
  std::string matrix_file = private_path + "/matrix";
  std::string bulletin_file = publish_path_ + "/bulletin";
  std::string sigma_file = public_path + "/sigma";
  std::string sigma_mkl_tree_file = public_path + "/sigma_mkl_tree";
  std::string vrf_pk_file = public_path + "/vrf_pk";
  std::string vrf_sk_file = private_path + "/vrf_sk";
  std::string key_meta_file = public_path + "/vrf_meta";

  if (!LoadBulletin(bulletin_file, bulletin_)) {
    assert(false);
    throw std::runtime_error("invalid bulletin file");
  }

  // vrf meta
  if (!LoadVrfMeta(key_meta_file, nullptr, vrf_meta_)) {
    assert(false);
    throw std::runtime_error("invalid vrf meta file");
  }

  // vrf bp
  vrf_key_bp_proofs_.resize(vrf_meta_.keys.size());
  for (size_t i = 0; i < vrf_key_bp_proofs_.size(); ++i) {
    auto const& key = vrf_meta_.keys[i];
    auto key_bp_file = public_path + "/key_bp_" + std::to_string(i);
    if (!LoadBpP1Proof(key_bp_file, key.bp_digest, vrf_key_bp_proofs_[i])) {
      assert(false);
      throw std::runtime_error("invalid key bp file");
    }
  }

  // key m
  key_m_.resize(vrf_meta_.keys.size());
  for (size_t i = 0; i < key_m_.size(); ++i) {
    auto& km = key_m_[i];
    auto key_m_file = public_path + "/key_m_" + std::to_string(i);    
    if (!LoadMatrix(key_m_file, bulletin_.n, km)) {
      assert(false);
      throw std::runtime_error("invalid key m file");
    }
  }

  // vrf sk
  if (!LoadVrfSk(vrf_sk_file, vrf_sk_)) {
    assert(false);
    throw std::runtime_error("invalid vrf sk file");
  }

  // vrf pk
  if (!LoadVrfPk(vrf_pk_file, nullptr, vrf_pk_)) {
    assert(false);
    throw std::runtime_error("invalid vrf pk file");
  }

  // sigma
  if (!LoadSigma(sigma_file, bulletin_.n, nullptr, sigmas_)) {
    assert(false);
    throw std::runtime_error("invalid sigma file");
  }

  // sigma mkl tree
  if (!LoadMkl(sigma_mkl_tree_file, bulletin_.n, sigma_mkl_tree_)) {
    assert(false);
    throw std::runtime_error("invalid sigma mkl tree file");
  }

  // matrix
  if (!LoadMatrix(matrix_file, bulletin_.n * bulletin_.s, m_)) {
    assert(false);
    throw std::runtime_error("invalid matrix file");
  }
}

VrfKeyMeta const* A::GetKeyMetaByName(std::string const& name) {
  auto const& names = vrf_meta_.column_names;
  auto it_name = std::find(names.begin(),names.end(), name);
  if (it_name == names.end()) return nullptr;
  auto col_index = std::distance(names.begin(), it_name);

  for (uint64_t i = 0; i < vrf_meta_.keys.size(); ++i) {
    auto const& vrf_key = vrf_meta_.keys[i];
    if (vrf_meta_.keys[i].column_index == (uint64_t)col_index) 
      return &vrf_key;
  }
  return nullptr;
}

}  // namespace scheme_table