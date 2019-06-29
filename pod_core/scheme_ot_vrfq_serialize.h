#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_ot_vrfq_protocol.h"

namespace scheme::table::ot_vrfq {
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
  ar &YAS_OBJECT_NVP("Request", ("k", t.key_name),
                     ("s", t.shuffled_value_digests), ("ot_vi", t.ot_vi),
                     ("ot_v", t.ot_v));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("Request", ("k", t.key_name),
                     ("s", t.shuffled_value_digests), ("ot_vi", t.ot_vi),
                     ("ot_v", t.ot_v));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("Response", ("s", t.shuffled_psk_exp_r), ("g", t.g_exp_r),
                     ("o", t.ot_ui));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("Response", ("s", t.shuffled_psk_exp_r), ("g", t.g_exp_r),
                     ("o", t.ot_ui));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("g", t.g_exp_r));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("g", t.g_exp_r));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  ar &YAS_OBJECT_NVP("Secret", ("r", t.r));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  ar &YAS_OBJECT_NVP("Secret", ("r", t.r));
}
}  // namespace scheme::table::ot_vrfq