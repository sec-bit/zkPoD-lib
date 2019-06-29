#pragma once

#include <stdint.h>
#include <string>
#include <vector>

#include "ecc.h"

namespace scheme::plain {

uint64_t GetDataBlockCount(uint64_t size, uint64_t column_num);

bool DataToM(std::string const& pathname, uint64_t size, uint64_t n,
             uint64_t column_num, std::vector<Fr>& m);

bool DecryptedRangeMToFile(std::string const& file, uint64_t size, uint64_t s,
                           uint64_t start, uint64_t count,
                           std::vector<Fr>::const_iterator m_begin,
                           std::vector<Fr>::const_iterator m_end);

}  // namespace scheme::plain