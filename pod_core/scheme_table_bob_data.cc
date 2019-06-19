#include "scheme_table_bob_data.h"
#include "misc.h"
#include "scheme_table.h"

namespace scheme::table {

BobData::BobData(Bulletin const& bulletin, std::string const& public_path)
    : bulletin_(bulletin), public_path_(public_path) {
  LoadData();
}

BobData::BobData(std::string const& bulletin_file, std::string const& public_path)
    : public_path_(public_path) {
  if (!LoadBulletin(bulletin_file, bulletin_))
    throw std::runtime_error("invalid bulletin file");
  LoadData();
}

// throw
bool BobData::NeedVerify() {
  bool verify = false;
  std::string verify_file = public_path_ + "/.verify";
  if (fs::is_regular_file(verify_file)) {
    time_t verify_time = fs::last_write_time(verify_file);
    auto range =
        boost::make_iterator_range(fs::directory_iterator(public_path_), {});
    for (auto& entry : range) {
      time_t file_time = fs::last_write_time(entry);
      if (file_time > verify_time) {
        verify = true;
        break;
      }
    }
  } else {
    verify = true;
  }
  return verify;
}

// throw
void BobData::LoadData() {
  if (!bulletin_.n || !bulletin_.s)
    throw std::runtime_error("invalid bulletin");

  std::string verify_file = public_path_ + "/.verify";
  std::string sigma_file = public_path_ + "/sigma";
  std::string sigma_mkl_tree_file = public_path_ + "/sigma_mkl_tree";
  std::string vrf_pk_file = public_path_ + "/vrf_pk";
  std::string key_meta_file = public_path_ + "/vrf_meta";
  h256_t const* check_h;

  bool verify = NeedVerify();

  // vrf meta
  check_h = verify ? &bulletin_.vrf_meta_digest : nullptr;
  if (!LoadVrfMeta(key_meta_file, check_h, vrf_meta_)) {
    assert(false);
    throw std::runtime_error("invalid vrf meta file");
  }

  // key m
  key_m_.resize(vrf_meta_.keys.size());
  for (size_t j = 0; j < key_m_.size(); ++j) {
    auto& km = key_m_[j];
    auto key_m_file = public_path_ + "/key_m_" + std::to_string(j);
    check_h = verify ? &vrf_meta_.keys[j].mj_mkl_root : nullptr;
    if (!LoadKeyM(key_m_file, bulletin_.n, vrf_meta_.keys[j].unique, check_h,
                  km)) {
      assert(false);
      throw std::runtime_error("invalid key m file");
    }
  }

  // sigma
  check_h = verify ? &bulletin_.sigma_mkl_root : nullptr;
  if (!LoadSigma(sigma_file, bulletin_.n, check_h, sigmas_)) {
    assert(false);
    throw std::runtime_error("invalid sigma file");
  }

  // vrf bp
  if (verify) {
    for (size_t j = 0; j < vrf_meta_.keys.size(); ++j) {
      auto key_bp_file = public_path_ + "/key_bp_" + std::to_string(j);
      if (!VerifyKeyBp(key_bp_file, bulletin_, vrf_meta_, j, key_m_[j],
                       sigmas_)) {
        assert(false);
        throw std::runtime_error("invalid key bp file");
      }
    }
  }

  // vrf pk
  check_h = verify ? &vrf_meta_.pk_digest : nullptr;
  if (!LoadVrfPk(vrf_pk_file, check_h, vrf_pk_)) {
    assert(false);
    throw std::runtime_error("invalid vrf pk file");
  }

  if (verify) {
    fs::remove(verify_file);
    fs::ofstream dummy(verify_file);
  }
}

bool BobData::SaveDecryped(std::string const& file, std::vector<Range> const& demands,
                  std::vector<Fr> const& decrypted) {
  Tick _tick_(__FUNCTION__);

  return DecryptedRangeMToFile(file, bulletin_.s, vrf_meta_, demands,
                               decrypted);
}
}  // namespace scheme::table