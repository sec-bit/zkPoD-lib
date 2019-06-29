#include "scheme_plain_alice_data.h"
#include "scheme_misc.h"
#include "scheme_plain.h"

namespace {}

namespace scheme::plain {

AliceData::AliceData(std::string const& publish_path)
    : publish_path_(publish_path) {
  std::string public_path = publish_path_ + "/public";
  std::string private_path = publish_path_ + "/private";
  std::string bulletin_file = publish_path_ + "/bulletin";
  std::string sigma_file = public_path + "/sigma";
  std::string sigma_mkl_tree_file = public_path + "/sigma_mkl_tree";
  std::string matrix_file = private_path + "/matrix";

  if (!LoadBulletin(bulletin_file, bulletin_)) {
    assert(false);
    throw std::runtime_error("Alice: invalid bulletin file");
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

  //// check sigma
  // auto check_sigma = CalcSigma(m_, bulletin_.n, bulletin_.s);
  // if (check_sigma != sigmas_) {
  //  assert(false);
  //  throw std::runtime_error("invalid sigma");
  //}
}

}  // namespace scheme::plain
