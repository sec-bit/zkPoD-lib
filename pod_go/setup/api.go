package setup

/*
#cgo CFLAGS: -I../..
#cgo LDFLAGS: -lpod_core

#include <stdlib.h>
#include "pod_core/capi/c_api.h"
*/
import "C"

import (
	"fmt"
	"unsafe"
)

// Load provides the Go interface for E_InitAll().
//
// XXX: Init() must have been called.
func Load(setupPubDir string) bool {
	setupPubCDir := C.CString(setupPubDir)
	defer C.free(unsafe.Pointer(setupPubCDir))
	fmt.Printf("setupPubDir=%v\n", setupPubDir)
	return bool(C.E_InitAll(setupPubCDir))
}
