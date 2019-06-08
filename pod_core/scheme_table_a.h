#pragma once

#include <stdint.h>
#include <string>
#include <memory>

#include "basic_types.h"
#include "bp.h"
#include "bulletin_table.h"
#include "mkl_tree.h"
#include "scheme_misc.h"
#include "vrf.h"
#include "vrf_meta.h"

namespace scheme::table {

class A {
 public:
  A(std::string const& publish_path);
  Bulletin const& bulletin() const { return bulletin_; }
  VrfMeta const& vrf_meta() const { return vrf_meta_; }
  vrf::Pk<> const& vrf_pk() const { return vrf_pk_; }
  vrf::Sk<> const& vrf_sk() const { return vrf_sk_; }
  std::vector<G1> const& sigmas() const { return sigmas_; }
  std::vector<Fr> const& m() const { return m_; }

 public:
  VrfKeyMeta const* GetKeyMetaByName(std::string const& name);

 private:
  std::string const publish_path_;
  scheme::table::Bulletin bulletin_;
  vrf::Pk<> vrf_pk_;
  vrf::Sk<> vrf_sk_;  // secret
  VrfMeta vrf_meta_;
  std::vector<bp::P1Proof> vrf_key_bp_proofs_;
  std::vector<G1> sigmas_;
  mkl::Tree sigma_mkl_tree_;
  std::vector<Fr> m_;  // secret
  std::vector<std::vector<Fr>> key_m_;

};

typedef std::shared_ptr<A> APtr;
}  // namespace scheme::table