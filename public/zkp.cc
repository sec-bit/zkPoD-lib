#include "zkp.h"
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <fstream>
#include <iostream>
#include <libff/common/profiling.hpp>
#include <sstream>
#include <vector>

namespace
{
void DisableLibffLog()
{
  libff::inhibit_profiling_info = true;
  libff::inhibit_profiling_counters = true;
}

struct imembuf : public std::streambuf
{
  imembuf(char const *array, size_t len)
  {
    char *p(const_cast<char *>(array));
    this->setg(p, p, p + len);
  }
};

struct imemstream : virtual imembuf, std::istream
{
  imemstream(char *array, size_t len)
      : imembuf(array, len), std::istream(static_cast<std::streambuf *>(this)) {}
};

struct omembuf : public std::streambuf
{
  omembuf(char *array, size_t len) { this->setp(array, array + len); }
};

struct omemstream : virtual omembuf, std::ostream
{
  omemstream(char *array, size_t len)
      : omembuf(array, len), std::ostream(static_cast<std::streambuf *>(this)) {}
};
} // namespace

void InitZkp(bool disable_log)
{
  libsnark::default_r1cs_gg_ppzksnark_pp::init_public_params();
  if (disable_log)
  {
    DisableLibffLog();
  }
}

ZkFr ConvertToZkFr(Fr const &mcl_fr)
{
  mpz_class m = mcl_fr.getMpz();
  return ZkFr(libff::bigint<ZkFr::num_limbs>(m.get_mpz_t()));
}

std::vector<ZkFr> ConvertToZkFr(std::vector<Fr> const &mcl_frs)
{
  std::vector<ZkFr> zk_frs(mcl_frs.size());
  for (size_t i = 0; i < zk_frs.size(); ++i)
  {
    zk_frs[i] = ConvertToZkFr(mcl_frs[i]);
  }
  return zk_frs;
}

std::vector<ZkFr> ConvertToZkFr(std::vector<uint64_t> const &o)
{
  std::vector<ZkFr> zk_frs(o.size());
  for (size_t i = 0; i < zk_frs.size(); ++i)
  {
    zk_frs[i] = ConvertToZkFr(Fr(o[i]));
  }
  return zk_frs;
}

ZkPkPtr LoadZkPk(std::string const &file)
{
  try
  {
    ZkPkPtr ret(new ZkPk());
    std::ifstream ifs;
    ifs.open(file, std::ifstream::in | std::ifstream::binary);
    ifs >> (*ret);
    return ret;
  }
  catch (std::exception &ex)
  {
    std::cerr << "Exception: " << ex.what() << "\n";
    return ZkPkPtr();
  }
}

ZkVkPtr LoadZkVk(std::string const &file)
{
  try
  {
    ZkVkPtr ret(new ZkVk());
    std::ifstream ifs;
    ifs.open(file, std::ifstream::in | std::ifstream::binary);
    ifs >> (*ret);
    return ret;
  }
  catch (std::exception &ex)
  {
    std::cerr << "Exception: " << ex.what() << "\n";
    return ZkVkPtr();
  }
}

void ZkProofToBin(ZkProof const &proof,
                  std::array<uint8_t, kZkProofSerializeSize> &bin)
{
  omemstream out((char *)bin.data(), bin.size());
  out << proof;
}

void ZkProofFromBin(ZkProof &proof,
                    std::array<uint8_t, kZkProofSerializeSize> const &bin)
{
  imemstream in((char *)bin.data(), bin.size());
  in >> proof;
}