package otbatch

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

	"github.com/sec-bit/zkPoD-lib/pod-go/table"
	"github.com/sec-bit/zkPoD-lib/pod-go/types"
	"github.com/sec-bit/zkPoD-lib/pod-go/utils"
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

// NewSellerSession provides the Go interface for E_TableOtBatchSessionNew().
func NewSellerSession(
	publishPath string, sellerID, buyerID [32]uint8,
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
		C.E_TableOtBatchSessionNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_TableOtBatchSessionNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &SellerSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_TableOtBatchSessionFree()
func (session *SellerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableOtBatchSessionFree(handle))
	if !ret {
		return fmt.Errorf("E_TableOtBatchSessionFree(%v) failed", handle)
	}
	return session.a.Free()
}

// GetNegoRequest provides the Go interface for E_TableOtBatchSessionGetNegoRequest()
func (session *SellerSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtBatchSessionGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtBatchSessionGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_TableOtBatchSessionOnNegoRequest()
func (session *SellerSession) OnNegoRequest(requestFile, responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(requestFile); err != nil {
		return err
	}
	if err := utils.CheckDirOfPathExistence(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtBatchSessionOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtBatchSessionOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_TableOtBatchSessionOnNegoResponse()
func (session *SellerSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtBatchSessionOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtBatchSessionOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnRequest provides the Go interface for E_TableOtBatchSessionOnRequest().
func (session *SellerSession) OnRequest(requestFile, responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(requestFile); err != nil {
		return err
	}
	if err := utils.CheckDirOfPathExistence(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	ret := bool(C.E_TableOtBatchSessionOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtBatchSessionOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_TableOtBatchSessionOnReceipt()
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

	ret := bool(C.E_TableOtBatchSessionOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtBatchSessionOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBuyerSession provides the Go interface for E_TableOtBatchClientNew()
func NewBuyerSession(
	bulletinFile, publicPath string,
	sellerID, buyerID [32]uint8, demands, phantoms []types.Range,
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
	nrPhantoms := len(phantoms)

	cDemands := make([]C.range_t, nrDemands)
	for idx, demand := range demands {
		cDemands[idx].start = C.uint64_t(demand.Start)
		cDemands[idx].count = C.uint64_t(demand.Count)
	}
	cDemandsPtr := (*C.range_t)(unsafe.Pointer(&cDemands[0]))

	cPhantoms := make([]C.range_t, nrPhantoms)
	for idx, phantom := range phantoms {
		cPhantoms[idx].start = C.uint64_t(phantom.Start)
		cPhantoms[idx].count = C.uint64_t(phantom.Count)
	}
	cPhantomsPtr := (*C.range_t)(unsafe.Pointer(&cPhantoms[0]))

	handle := C.handle_t(b.CHandle())
	session := types.CHandle(
		C.E_TableOtBatchClientNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			cDemandsPtr,
			C.uint64_t(nrDemands),
			cPhantomsPtr,
			C.uint64_t(nrPhantoms)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_TableOtBatchClientNew(%v, %v, %v, %v, %d, %v, %d) failed",
			handle, buyerID, sellerID,
			demands, nrDemands, phantoms, nrPhantoms)
	}

	return &BuyerSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_TableOtBatchClientFree()
func (session *BuyerSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableOtBatchClientFree(handle))
	if !ret {
		return fmt.Errorf("E_TableOtBatchClientFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetNegoRequest provides the Go interface for E_TableOtBatchClientGetNegoRequest()
func (session *BuyerSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtBatchClientGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtBatchClientGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_TableOtBatchClientOnNegoResponse()
func (session *BuyerSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtBatchClientOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtBatchClientOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_TableOtBatchClientOnNegoRequest()
func (session *BuyerSession) OnNegoRequest(requestFile, responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(requestFile); err != nil {
		return err
	}
	if err := utils.CheckDirOfPathExistence(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtBatchClientOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtBatchClientOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// GetRequest provides the Go interface for E_TableOtBatchClientGetRequest()
func (session *BuyerSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtBatchClientGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtBatchClientGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_TableOtBatchClientOnResponse()
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

	ret := bool(C.E_TableOtBatchClientOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtBatchClientOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_TableOtBatchClientOnSecret()
func (session *BuyerSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_TableOtBatchClientOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtBatchClientOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_TableOtBatchClientSaveDecrypted()
func (session *BuyerSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_TableOtBatchClientSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtBatchClientSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}

// GenerateClaim provides the Go interface for E_TableOtBatchClientGenerateClaim()
func (session *BuyerSession) GenerateClaim(claimFile string) error {
	if err := utils.CheckDirOfPathExistence(claimFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	claimFileCStr := C.CString(claimFile)
	defer C.free(unsafe.Pointer(claimFileCStr))

	ret := bool(C.E_TableOtBatchClientGenerateClaim(handle, claimFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtBatchClientGenerateClaim(%v, %s) failed",
			handle, claimFile)
	}

	return nil
}
