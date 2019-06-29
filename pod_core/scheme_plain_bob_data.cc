#include "scheme_plain_bob_data.h"
#include "misc.h"
#include "scheme_plain.h"

namespace scheme::plain {
BobData::BobData(Bulletin const& bulletin, std::string const& public_path)
    : bulletin_(bulletin), public_path_(public_path) {
  LoadData();
}

BobData::BobData(std::string const& bulletin_file,
                 std::string const& public_path)
    : public_path_(public_path) {
  if (!LoadBulletin(bulletin_file, bulletin_))
    throw std::runtime_error("Bob: invalid bulletin file");
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
    throw std::runtime_error("Bob: invalid bulletin");

  std::string verify_file = public_path_ + "/.verify";
  std::string sigma_file = public_path_ + "/sigma";

  h256_t const* check_h;
  bool verify = NeedVerify();

  // sigma
  check_h = verify ? &bulletin_.sigma_mkl_root : nullptr;
  if (!LoadSigma(sigma_file, bulletin_.n, check_h, sigmas_)) {
    assert(false);
    throw std::runtime_error("invalid sigma file");
  }

  if (verify) {
    fs::remove(verify_file);
    fs::ofstream dummy(verify_file);
  }
}

bool BobData::SaveDecryped(std::string const& file,
                           std::vector<Range> const& demands,
                           std::vector<Fr> const& decrypted) {
  Tick _tick_(__FUNCTION__);
  std::vector<Fr>::const_iterator m_begin = decrypted.begin();
  std::vector<Fr>::const_iterator m_end;
  for (auto const& demand : demands) {
    std::string range_file = file + "_" + std::to_string(demand.start) + "_" +
                             std::to_string(demand.count);
    m_end = m_begin + demand.count * bulletin_.s;
    if (!DecryptedRangeMToFile(range_file, bulletin_.size, bulletin_.s,
                               demand.start, demand.count, m_begin, m_end)) {
      assert(false);
      return false;
    }
    m_begin = m_end;
  }
  return true;
}
}  // namespace scheme::plain