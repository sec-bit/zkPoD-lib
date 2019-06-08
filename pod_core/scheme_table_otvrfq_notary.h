#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_table_otvrfq_protocol.h"

namespace scheme::table::otvrfq {
bool VerifyProof(G1 const& g_exp_r, Secret const& secret);
bool VerifyProof(Receipt const& receipt, Secret const& secret);
}  // namespace scheme::table::otvrfq