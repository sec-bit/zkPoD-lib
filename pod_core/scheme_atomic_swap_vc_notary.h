#pragma once

#include <memory>
#include <string>
#include "basic_types.h"
#include "scheme_atomic_swap_vc_protocol.h"

namespace scheme::atomic_swap_vc {
bool VerifyProof(Fr const& seed0, Fr const& seed0_rand, Fr const& digest);
}  // namespace scheme::atomic_swap_vc
