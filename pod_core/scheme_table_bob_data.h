#pragma once

#include <stdint.h>
#include <string>

#include "basic_types.h"
#include "bp.h"
#include "bulletin_table.h"
#include "mkl_tree.h"
#include "scheme_misc.h"
#include "vrf.h"
#include "vrf_meta.h"

namespace scheme::table {

class BobData {
 public:
  BobData(Bulletin const& bulletin, std::string const& public_path);
  BobData(std::string const& bulletin_file, std::string const& public_path);
  Bulletin const& bulletin() const { return bulletin_; }
  vrf::Pk<> const& vrf_pk() const { return vrf_pk_; }
  VrfMeta const& vrf_meta() const { return vrf_meta_; }
  std::vector<G1> sigmas() const { return sigmas_; }
  std::vector<std::vector<Fr>> const& key_m() const { return key_m_; }
  bool SaveDecryped(std::string const& file, std::vector<Range> const& demands,
                    std::vector<Fr> const& decrypted);
 private:
  void LoadData();
  bool NeedVerify();

 private:
  Bulletin bulletin_;
  std::string public_path_;

 private:
  VrfMeta vrf_meta_;
  vrf::Pk<> vrf_pk_;
  std::vector<G1> sigmas_;
  std::vector<std::vector<Fr>> key_m_;
};

typedef std::shared_ptr<BobData> BobDataPtr;
}  // namespace scheme::table