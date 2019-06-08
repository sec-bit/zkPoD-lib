#pragma once

#include "basic_types_serialize.h"
#include "misc.h"
#include "scheme_batch3_protocol.h"

namespace scheme::batch3 {
// save to bin
template <typename Ar>
void serialize(Ar &ar, Request const &t) {
  ar &YAS_OBJECT_NVP("Request", ("d", t.demands));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Request &t) {
  ar &YAS_OBJECT_NVP("Request", ("d", t.demands));
}

// save to bin
template <typename Ar>
void serialize(Ar &ar, Response const &t) {
  ar &YAS_OBJECT_NVP("spb3::Response", ("uk", t.uk), ("ux0", t.ux0),
                     ("u0x", t.u0x), ("g2x0", t.g2x0), ("ud", t.ud),
                     ("g2d", t.g2d), ("m", t.m), ("ek", t.ek), ("ex", t.ex));
}

// load from bin
template <typename Ar>
void serialize(Ar &ar, Response &t) {
  ar &YAS_OBJECT_NVP("spb3::Response", ("uk", t.uk), ("ux0", t.ux0),
                     ("u0x", t.u0x), ("g2x0", t.g2x0), ("ud", t.ud),
                     ("g2d", t.g2d), ("m", t.m), ("ek", t.ek), ("ex", t.ex));
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
}  // namespace scheme::batch3
