package atomic_swap_vc

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

// NewAliceSession provides the Go interface for E_PlainAtomicSwapVcAliceNew().
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
		C.E_PlainAtomicSwapVcAliceNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_PlainAtomicSwapVcAliceNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &AliceSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_PlainAtomicSwapVcAliceFree()
func (session *AliceSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainAtomicSwapVcAliceFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainAtomicSwapVcAliceFree(%v) failed", handle)
	}
	return session.a.Free()
}

// OnRequest provides the Go interface for E_PlainAtomicSwapVcAliceOnRequest().
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

	ret := bool(C.E_PlainAtomicSwapVcAliceOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapVcAliceOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_PlainAtomicSwapVcAliceOnReceipt()
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

	ret := bool(C.E_PlainAtomicSwapVcAliceOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapVcAliceOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBobSession provides the Go interface for E_PlainAtomicSwapVcBobNew()
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
		C.E_PlainAtomicSwapVcBobNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			cDemandsPtr,
			C.uint64_t(nrDemands)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_PlainAtomicSwapVcBobNew(%v, %v, %v, %v, %d) failed",
			handle, buyerID, sellerID, demands, nrDemands)
	}

	return &BobSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_PlainAtomicSwapVcBobFree()
func (session *BobSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainAtomicSwapVcBobFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainAtomicSwapVcBobFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetRequest provides the Go interface for E_PlainAtomicSwapVcBobGetRequest()
func (session *BobSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_PlainAtomicSwapVcBobGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_PlainAtomicSwapVcBobGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_PlainAtomicSwapVcBobOnResponse()
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

	ret := bool(C.E_PlainAtomicSwapVcBobOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapVcBobOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_PlainAtomicSwapVcBobOnSecret()
func (session *BobSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_PlainAtomicSwapVcBobOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapVcBobOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_PlainAtomicSwapVcBobSaveDecrypted()
func (session *BobSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_PlainAtomicSwapVcBobSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainAtomicSwapVcBobSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}
