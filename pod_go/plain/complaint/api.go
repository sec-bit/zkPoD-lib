package complaint

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
	SellerSession struct {
		a      *plain.A
		handle types.CHandle
	}

	BuyerSession struct {
		b      *plain.B
		handle types.CHandle
	}
)

// NewSellerSession provides the Go interface for E_PlainComplaintSessionNew().
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
		C.E_PlainComplaintSessionNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_PlainComplaintSessionNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &SellerSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_PlainComplaintSessionFree()
func (session *SellerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainComplaintSessionFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainComplaintSessionFree(%v) failed", handle)
	}
	return session.a.Free()
}

// OnRequest provides the Go interface for E_PlainComplaintSessionOnRequest().
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

	ret := bool(C.E_PlainComplaintSessionOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainComplaintSessionOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_PlainComplaintSessionOnReceipt()
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

	ret := bool(C.E_PlainComplaintSessionOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainComplaintSessionOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBuyerSession provides the Go interface for E_PlainComplaintClientNew()
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
		C.E_PlainComplaintClientNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			cDemandsPtr,
			C.uint64_t(nrDemands)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_PlainComplaintClientNew(%v, %v, %v, %v, %d) failed",
			handle, buyerID, sellerID, demands, nrDemands)
	}

	return &BuyerSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_PlainComplaintClientFree()
func (session *BuyerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainComplaintClientFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainComplaintClientFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetRequest provides the Go interface for E_PlainComplaintClientGetRequest()
func (session *BuyerSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_PlainComplaintClientGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_PlainComplaintClientGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_PlainComplaintClientOnResponse()
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

	ret := bool(C.E_PlainComplaintClientOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainComplaintClientOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_PlainComplaintClientOnSecret()
func (session *BuyerSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_PlainComplaintClientOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainComplaintClientOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_PlainComplaintClientSaveDecrypted()
func (session *BuyerSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_PlainComplaintClientSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainComplaintClientSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}

// GenerateClaim provides the Go interface for E_PlainComplaintClientGenerateClaim()
func (session *BuyerSession) GenerateClaim(claimFile string) error {
	if err := utils.CheckDirOfPathExistence(claimFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	claimFileCStr := C.CString(claimFile)
	defer C.free(unsafe.Pointer(claimFileCStr))

	ret := bool(C.E_PlainComplaintClientGenerateClaim(handle, claimFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainComplaintClientGenerateClaim(%v, %s) failed",
			handle, claimFile)
	}

	return nil
}
