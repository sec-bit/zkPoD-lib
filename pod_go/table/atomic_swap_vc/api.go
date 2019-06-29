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

	"github.com/sec-bit/zkPoD-lib/pod_go/table"
	"github.com/sec-bit/zkPoD-lib/pod_go/types"
	"github.com/sec-bit/zkPoD-lib/pod_go/utils"
)

type (
	AliceSession struct {
		a      *table.A
		handle types.CHandle
	}

	BobSession struct {
		b      *table.B
		handle types.CHandle
	}
)

// NewAliceSession provides the Go interface for E_TableAtomicSwapVcAliceNew().
func NewAliceSession(
	publishPath string, sellerID, buyerID [40]uint8,
) (*AliceSession, error) {
	a, err := table.NewA(publishPath)
	if err != nil {
		return nil, err
	}

	sellerIDCPtr := types.NewUInt8(sellerID[:])
	defer sellerIDCPtr.Free()

	buyerIDCPtr := types.NewUInt8(buyerID[:])
	defer buyerIDCPtr.Free()

	handle := C.handle_t(a.CHandle())
	session := types.CHandle(
		C.E_TableAtomicSwapVcAliceNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_TableAtomicSwapVcAliceNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &AliceSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_TableAtomicSwapVcAliceFree()
func (session *AliceSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableAtomicSwapVcAliceFree(handle))
	if !ret {
		return fmt.Errorf("E_TableAtomicSwapVcAliceFree(%v) failed", handle)
	}
	return session.a.Free()
}

// OnRequest provides the Go interface for E_TableAtomicSwapVcAliceOnRequest().
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

	ret := bool(C.E_TableAtomicSwapVcAliceOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapVcAliceOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_TableAtomicSwapVcAliceOnReceipt()
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

	ret := bool(C.E_TableAtomicSwapVcAliceOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapVcAliceOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBobSession provides the Go interface for E_TableAtomicSwapVcBobNew()
func NewBobSession(
	bulletinFile, publicPath string,
	sellerID, buyerID [40]uint8, demands []types.Range,
) (*BobSession, error) {
	b, err := table.NewB(bulletinFile, publicPath)
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
		C.E_TableAtomicSwapVcBobNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			cDemandsPtr,
			C.uint64_t(nrDemands)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_TableAtomicSwapVcBobNew(%v, %v, %v, %v, %d) failed",
			handle, buyerID, sellerID, demands, nrDemands)
	}

	return &BobSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_TableAtomicSwapVcBobFree()
func (session *BobSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableAtomicSwapVcBobFree(handle))
	if !ret {
		return fmt.Errorf("E_TableAtomicSwapVcBobFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetRequest provides the Go interface for E_TableAtomicSwapVcBobGetRequest()
func (session *BobSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableAtomicSwapVcBobGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableAtomicSwapVcBobGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_TableAtomicSwapVcBobOnResponse()
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

	ret := bool(C.E_TableAtomicSwapVcBobOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapVcBobOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_TableAtomicSwapVcBobOnSecret()
func (session *BobSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_TableAtomicSwapVcBobOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapVcBobOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_TableAtomicSwapVcBobSaveDecrypted()
func (session *BobSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_TableAtomicSwapVcBobSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapVcBobSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}
