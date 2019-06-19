#include "c_api.h"
#include "ecc.h"
#include "ecc_pub.h"

extern "C" {
EXPORT void E_InitEcc() { InitEcc(); }

EXPORT bool E_LoadEccPub(char const* ecc_pub_file) {
  return LoadEccPub(ecc_pub_file);
}
}  // extern "C"