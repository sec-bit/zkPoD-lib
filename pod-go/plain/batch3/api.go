package batch3

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

	"github.com/sec-bit/zkPoD-lib/pod-go/plain"
	"github.com/sec-bit/zkPoD-lib/pod-go/types"
	"github.com/sec-bit/zkPoD-lib/pod-go/utils"
)

type (
	SellerSession struct {
		a      *plain.A
		handle types.CHandle
	}

	BuyerSession struct {
		b      *plain.B
		handle types.CHandle
	}
)

// NewSellerSession provides the Go interface for E_PlainBatch3SessionNew().
func NewSellerSession(
	publishPath string, sellerID, buyerID [32]uint8,
) (*SellerSession, error) {
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
		C.E_PlainBatch3SessionNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_PlainBatch3SessionNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &SellerSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_PlainBatch3SessionFree()
func (session *SellerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainBatch3SessionFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainBatch3SessionFree(%v) failed", handle)
	}
	return session.a.Free()
}

// OnRequest provides the Go interface for E_PlainBatch3SessionOnRequest().
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

	ret := bool(C.E_PlainBatch3SessionOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainBatch3SessionOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_PlainBatch3SessionOnReceipt()
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

	ret := bool(C.E_PlainBatch3SessionOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainBatch3SessionOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBuyerSession provides the Go interface for E_PlainBatch3ClientNew()
func NewBuyerSession(
	bulletinFile, publicPath string,
	sellerID, buyerID [32]uint8, demands []types.Range,
) (*BuyerSession, error) {
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
		C.E_PlainBatch3ClientNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			cDemandsPtr,
			C.uint64_t(nrDemands)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_PlainBatch3ClientNew(%v, %v, %v, %v, %d) failed",
			handle, buyerID, sellerID, demands, nrDemands)
	}

	return &BuyerSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_PlainBatch3ClientFree()
func (session *BuyerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainBatch3ClientFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainBatch3ClientFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetRequest provides the Go interface for E_PlainBatch3ClientGetRequest()
func (session *BuyerSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_PlainBatch3ClientGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_PlainBatch3ClientGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_PlainBatch3ClientOnResponse()
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

	ret := bool(C.E_PlainBatch3ClientOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainBatch3ClientOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_PlainBatch3ClientOnSecret()
func (session *BuyerSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_PlainBatch3ClientOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainBatch3ClientOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_PlainBatch3ClientSaveDecrypted()
func (session *BuyerSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_PlainBatch3ClientSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainBatch3ClientSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}
