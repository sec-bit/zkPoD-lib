#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_atomic_swap_vc_protocol.h"

namespace scheme::atomic_swap_vc {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("Request", ("s", t.seed2_seed), ("p", t.demands));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("Request", ("s", t.seed2_seed), ("p", t.demands));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("Response", ("k", t.k), ("m", t.m), ("vw", t.vw),
                     ("seed0_mimc3_digest", t.seed0_mimc3_digest),
                     ("zk_ip_vw", t.zk_ip_vw), ("zk_proofs", t.zk_proofs));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("Response", ("k", t.k), ("m", t.m), ("vw", t.vw),
                     ("seed0_mimc3_digest", t.seed0_mimc3_digest),
                     ("zk_ip_vw", t.zk_ip_vw), ("zk_proofs", t.zk_proofs));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("d", t.seed0_mimc3_digest));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("d", t.seed0_mimc3_digest));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("Secret", ("s", t.seed0),("r", t.seed0_rand));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("Secret", ("s", t.seed0),("r", t.seed0_rand));
}
}  // namespace scheme::atomic_swap_vc
