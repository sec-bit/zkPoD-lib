#pragma once

#include <stdint.h>
#include <memory>
#include <string>
#include <vector>

#include <cryptopp/keccak.h>
#include <boost/endian/conversion.hpp>

#include "ecc.h"

G1 HashNameI(h256_t const& name, uint64_t i);

Fr ChainKeccak256(uint8_t const* seed_buf, uint64_t seed_len, uint64_t index);

Fr ChainKeccak256(h256_t const& seed, uint64_t index);

void ChainKeccak256(h256_t const& seed, uint64_t count, std::vector<Fr>& v);

Fr ChainMimcInv(Fr const& seed, uint64_t index);

void ChainMimcInv(Fr const& seed, uint64_t count, std::vector<Fr>& v);