package types

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

type (
	CHandle C.handle_t

	CUInt8 C.uint8_t

	Range struct {
		Start uint64
		Count uint64
	}
)

func NewUInt8(vals []uint8) *CUInt8 {
	n := len(vals)
	p := (*CUInt8)(C.calloc(C.size_t(n), C.size_t(unsafe.Sizeof(CUInt8(0)))))
	s := p.Slice(n)

	for i, v := range vals {
		s[i] = v
	}

	return p
}

func (p *CUInt8) Slice(n int) []uint8 {
	return ((*[1 << 31]uint8)(unsafe.Pointer(p)))[0:n:n]
}

func (p *CUInt8) Free() {
	C.free(unsafe.Pointer(p))
}

func (r Range) String() string {
	return fmt.Sprintf("Range{ Start: %d, Count: %d }", r.Start, r.Count)
}
