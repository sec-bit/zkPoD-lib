#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "basic_types.h"

namespace scheme {
namespace table {

struct Bulletin {
  uint64_t n;
  uint64_t s;
  h256_t sigma_mkl_root;
  h256_t vrf_meta_digest;
};

bool IsBulletinValid(Bulletin const& bulletin);

bool SaveBulletin(std::string const& output, Bulletin const& bulletin);

bool LoadBulletin(std::string const& input, Bulletin& bulletin);

}  // namespace table
}  // namespace scheme