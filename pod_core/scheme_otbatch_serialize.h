#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_otbatch_protocol.h"

namespace scheme::otbatch {
// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest const &t) {
  ar &YAS_OBJECT_NVP("NegoARequest", ("s", t.s));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoARequest &t) {
  ar &YAS_OBJECT_NVP("NegoARequest", ("s", t.s));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse const &t) {
  ar &YAS_OBJECT_NVP("NegoAResponse", ("s", t.s_exp_beta));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoAResponse &t) {
  ar &YAS_OBJECT_NVP("NegoAResponse", ("s", t.s_exp_beta));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest const &t) {
  ar &YAS_OBJECT_NVP("NegoBRequest", ("t", t.t));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBRequest &t) {
  ar &YAS_OBJECT_NVP("NegoBRequest", ("t", t.t));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse const &t) {
  ar &YAS_OBJECT_NVP("NegoBResponse", ("t", t.t_exp_alpha));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, NegoBResponse &t) {
  ar &YAS_OBJECT_NVP("NegoBResponse", ("t", t.t_exp_alpha));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("Request", ("s", t.seed2_seed), ("p", t.phantoms),
                     ("ot_vi", t.ot_vi), ("ot_v", t.ot_v));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("Request", ("s", t.seed2_seed), ("p", t.phantoms),
                     ("ot_vi", t.ot_vi), ("ot_v", t.ot_v));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("Response", ("k", t.k), ("o", t.ot_ui), ("m", t.m));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("Response", ("k", t.k), ("o", t.ot_ui), ("m", t.m));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("s", t.seed2), ("k", t.k_mkl_root),
                     ("c", t.count));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("Secret", ("s", t.seed0));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("Secret", ("s", t.seed0));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Claim const &t) {
  ar &YAS_OBJECT_NVP("Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Claim &t) {
  ar &YAS_OBJECT_NVP("Claim", ("i", t.i), ("j", t.j), ("k", t.kij),
                     ("m", t.mkl_path));
}

}  // namespace scheme::otbatch
