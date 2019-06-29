package atomic_swap

/*
#cgo CFLAGS: -I../../..
#cgo LDFLAGS: -lpod_core

#include <stdlib.h>
#include "pod_core/capi/c_api.h"
*/
import "C"

import (
	"fmt"
	"unsafe"

	"github.com/sec-bit/zkPoD-lib/pod_go/plain"
	"github.com/sec-bit/zkPoD-lib/pod_go/types"
	"github.com/sec-bit/zkPoD-lib/pod_go/utils"
)

type (
	AliceSession struct {
		a      *plain.A
		handle types.CHandle
	}

	BobSession struct {
		b      *plain.B
		handle types.CHandle
	}
)

// NewAliceSession provides the Go interface for E_PlainAtomicSwapAliceNew().
func NewAliceSession(
	publishPath string, sellerID, buyerID [40]uint8,
) (*AliceSession, error) {
	a, err := plain.NewA(publishPath)
	if err != nil {
		return nil, err
	}

	sellerIDCPtr := types.NewUInt8(sellerID[:])
	defer sellerIDCPtr.Free()

	buyerIDCPtr := types.NewUInt8(buyerID[:])
	defer buyerIDCPtr.Free()

	handle := C.handle_t(a.CHandle())
	session := types.CHandle(
		C.E_PlainAtomicSwapAliceNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_PlainAtomicSwapAliceNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &AliceSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_PlainAtomicSwapAliceFree()
func (session *AliceSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainAtomicSwapAliceFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainAtomicSwapAliceFree(%v) failed", handle)
	}
	return session.a.Free()
}

// OnRequest provides the Go interface for E_PlainAtomicSwapAliceOnRequest().
func (session *AliceSession) OnRequest(requestFile, responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(requestFile); err != nil {
		return err
	}
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	ret := bool(C.E_PlainAtomicSwapAliceOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapAliceOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_PlainAtomicSwapAliceOnReceipt()
func (session *AliceSession) OnReceipt(receiptFile, secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(receiptFile); err != nil {
		return err
	}
	if err := utils.CheckDirOfPathExistence(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	receiptFileCStr := C.CString(receiptFile)
	defer C.free(unsafe.Pointer(receiptFileCStr))

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_PlainAtomicSwapAliceOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapAliceOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBobSession provides the Go interface for E_PlainAtomicSwapBobNew()
func NewBobSession(
	bulletinFile, publicPath string,
	sellerID, buyerID [40]uint8, demands []types.Range,
) (*BobSession, error) {
	b, err := plain.NewB(bulletinFile, publicPath)
	if err != nil {
		return nil, err
	}

	sellerIDCPtr := types.NewUInt8(sellerID[:])
	defer sellerIDCPtr.Free()

	buyerIDCPtr := types.NewUInt8(buyerID[:])
	defer buyerIDCPtr.Free()

	nrDemands := len(demands)

	cDemands := make([]C.range_t, nrDemands)
	for idx, demand := range demands {
		cDemands[idx].start = C.uint64_t(demand.Start)
		cDemands[idx].count = C.uint64_t(demand.Count)
	}
	cDemandsPtr := (*C.range_t)(unsafe.Pointer(&cDemands[0]))

	handle := C.handle_t(b.CHandle())
	session := types.CHandle(
		C.E_PlainAtomicSwapBobNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			cDemandsPtr,
			C.uint64_t(nrDemands)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_PlainAtomicSwapBobNew(%v, %v, %v, %v, %d) failed",
			handle, buyerID, sellerID, demands, nrDemands)
	}

	return &BobSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_PlainAtomicSwapBobFree()
func (session *BobSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainAtomicSwapBobFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainAtomicSwapBobFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetRequest provides the Go interface for E_PlainAtomicSwapBobGetRequest()
func (session *BobSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_PlainAtomicSwapBobGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_PlainAtomicSwapBobGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_PlainAtomicSwapBobOnResponse()
func (session *BobSession) OnResponse(responseFile, receiptFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}
	if err := utils.CheckDirOfPathExistence(receiptFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	receiptFileCStr := C.CString(receiptFile)
	defer C.free(unsafe.Pointer(receiptFileCStr))

	ret := bool(C.E_PlainAtomicSwapBobOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapBobOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_PlainAtomicSwapBobOnSecret()
func (session *BobSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_PlainAtomicSwapBobOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapBobOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_PlainAtomicSwapBobSaveDecrypted()
func (session *BobSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_PlainAtomicSwapBobSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapBobSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}
