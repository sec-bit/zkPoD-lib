#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_vrfq_protocol.h"

namespace scheme::table::vrfq {
bool VerifyProof(G1 const& g_exp_r, Secret const& secret);
bool VerifyProof(Receipt const& receipt, Secret const& secret);
}  // namespace scheme::table::vrfq