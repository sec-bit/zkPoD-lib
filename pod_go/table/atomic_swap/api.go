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

	"github.com/sec-bit/zkPoD-lib/pod_go/table"
	"github.com/sec-bit/zkPoD-lib/pod_go/types"
	"github.com/sec-bit/zkPoD-lib/pod_go/utils"
)

type (
	SellerSession struct {
		a      *table.A
		handle types.CHandle
	}

	BuyerSession struct {
		b      *table.B
		handle types.CHandle
	}
)

// NewSellerSession provides the Go interface for E_TableAtomicSwapSessionNew().
func NewSellerSession(
	publishPath string, sellerID, buyerID [40]uint8,
) (*SellerSession, error) {
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
		C.E_TableAtomicSwapSessionNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_TableAtomicSwapSessionNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &SellerSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_TableAtomicSwapSessionFree()
func (session *SellerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableAtomicSwapSessionFree(handle))
	if !ret {
		return fmt.Errorf("E_TableAtomicSwapSessionFree(%v) failed", handle)
	}
	return session.a.Free()
}

// OnRequest provides the Go interface for E_TableAtomicSwapSessionOnRequest().
func (session *SellerSession) OnRequest(requestFile, responseFile string) error {
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

	ret := bool(C.E_TableAtomicSwapSessionOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapSessionOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_TableAtomicSwapSessionOnReceipt()
func (session *SellerSession) OnReceipt(receiptFile, secretFile string) error {
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

	ret := bool(C.E_TableAtomicSwapSessionOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapSessionOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBuyerSession provides the Go interface for E_TableAtomicSwapClientNew()
func NewBuyerSession(
	bulletinFile, publicPath string,
	sellerID, buyerID [40]uint8, demands []types.Range,
) (*BuyerSession, error) {
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
		C.E_TableAtomicSwapClientNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			cDemandsPtr,
			C.uint64_t(nrDemands)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_TableAtomicSwapClientNew(%v, %v, %v, %v, %d) failed",
			handle, buyerID, sellerID, demands, nrDemands)
	}

	return &BuyerSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_TableAtomicSwapClientFree()
func (session *BuyerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableAtomicSwapClientFree(handle))
	if !ret {
		return fmt.Errorf("E_TableAtomicSwapClientFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetRequest provides the Go interface for E_TableAtomicSwapClientGetRequest()
func (session *BuyerSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableAtomicSwapClientGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableAtomicSwapClientGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_TableAtomicSwapClientOnResponse()
func (session *BuyerSession) OnResponse(responseFile, receiptFile string) error {
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

	ret := bool(C.E_TableAtomicSwapClientOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapClientOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_TableAtomicSwapClientOnSecret()
func (session *BuyerSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_TableAtomicSwapClientOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapClientOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_TableAtomicSwapClientSaveDecrypted()
func (session *BuyerSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_TableAtomicSwapClientSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableAtomicSwapClientSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}
