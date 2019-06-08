#pragma once

#include "basic_types_serialize.h"
#include "scheme_otbatch3_protocol.h"

namespace scheme::otbatch3 {
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
  ar &YAS_OBJECT_NVP("Request", ("p", t.phantoms), ("ot_vi", t.ot_vi),
                     ("ot_v", t.ot_v));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("Request", ("p", t.phantoms), ("ot_vi", t.ot_vi),
                     ("ot_v", t.ot_v));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("spb3::Response", ("uk", t.uk), ("ux0", t.ux0),
                     ("u0x", t.u0x), ("g2x0", t.g2x0), ("ud", t.ud),
                     ("g2d", t.g2d), ("m", t.m), ("ek", t.ek), ("ex", t.ex),
                     ("o", t.ot_ui));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("spb3::Response", ("uk", t.uk), ("ux0", t.ux0),
                     ("u0x", t.u0x), ("g2x0", t.g2x0), ("ud", t.ud),
                     ("g2d", t.g2d), ("m", t.m), ("ek", t.ek), ("ex", t.ex),
                     ("o", t.ot_ui));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Receipt const &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("u0_x0_lgs", t.u0_x0_lgs), ("u0d", t.u0d));
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Receipt &t) {
  ar &YAS_OBJECT_NVP("Receipt", ("u0_x0_lgs", t.u0_x0_lgs), ("u0d", t.u0d));
}

// save to json
template <typename Ar>
void serialize(Ar &ar, Secret const &t) {
  //#ifdef _DEBUG
  //  ar &YAS_OBJECT_NVP("Secret", ("x0_lgs", t.x0_lgs), ("d", t.d),
  //                     ("k", t.k), ("m", t.m), ("x", t.x));
  //#else
  ar &YAS_OBJECT_NVP("Secret", ("x0_lgs", t.x0_lgs), ("d", t.d));
  //#endif
}

// load from json
template <typename Ar>
void serialize(Ar &ar, Secret &t) {
  //#ifdef _DEBUG
  //  ar &YAS_OBJECT_NVP("Secret", ("x0_lgs", t.x0_lgs), ("d", t.d),
  //                     ("k", t.k), ("m", t.m), ("x", t.x));
  //#else
  ar &YAS_OBJECT_NVP("Secret", ("x0_lgs", t.x0_lgs), ("d", t.d));
  //#endif
}
}  // namespace scheme::otbatch3
