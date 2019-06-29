#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "basic_types.h"

namespace scheme {
namespace plain {

struct Bulletin {
  uint64_t size;
  uint64_t s;
  uint64_t n;
  h256_t sigma_mkl_root;
};
bool IsBulletinValid(Bulletin const& bulletin);

bool SaveBulletin(std::string const& output, Bulletin const& bulletin);

bool LoadBulletin(std::string const& input, Bulletin& bulletin);

}  // namespace plain
}  // namespace scheme