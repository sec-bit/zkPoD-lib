package ecc

/*
#cgo CFLAGS: -I../..
#cgo LDFLAGS: -lpod_core

#include <stdlib.h>
#include "pod_core/capi/c_api.h"
*/
import "C"

import (
	"unsafe"
)

// Init provides Go interface for E_Init().
func Init() {
	C.E_InitEcc()
	return
}

// Load provides the Go interface for E_LoadEccPub().
//
// XXX: Init() must have been called.
func Load(eccPubFile string) bool {
	eccPubFileCStr := C.CString(eccPubFile)
	defer C.free(unsafe.Pointer(eccPubFileCStr))

	return bool(C.E_LoadEccPub(eccPubFileCStr))
}
