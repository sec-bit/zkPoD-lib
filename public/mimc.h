#pragma once

#include <cryptopp/keccak.h>
#include <vector>

#include "ecc.h"

enum {
  kMimc3Round = 64,
  kMimcInvRound = 64,
};

std::vector<Fr> const& Mimc3Const();

std::vector<Fr> const& MimcInvConst();

Fr Mimc3(Fr const& left, Fr const& right);

Fr Mimc3Circuit(Fr const& left, Fr const& right);

Fr MimcInv(Fr const& s);

Fr MimcInvCircuit(Fr const& s);