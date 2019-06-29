package ot_complaint

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

// NewAliceSession provides the Go interface for E_TableOtComplaintAliceNew().
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
		C.E_TableOtComplaintAliceNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_TableOtComplaintAliceNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &AliceSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_TableOtComplaintAliceFree()
func (session *AliceSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableOtComplaintAliceFree(handle))
	if !ret {
		return fmt.Errorf("E_TableOtComplaintAliceFree(%v) failed", handle)
	}
	return session.a.Free()
}

// GetNegoRequest provides the Go interface for E_TableOtComplaintAliceGetNegoRequest()
func (session *AliceSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtComplaintAliceGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtComplaintAliceGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_TableOtComplaintAliceOnNegoRequest()
func (session *AliceSession) OnNegoRequest(requestFile, responseFile string) error {
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

	if ret := bool(C.E_TableOtComplaintAliceOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtComplaintAliceOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_TableOtComplaintAliceOnNegoResponse()
func (session *AliceSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtComplaintAliceOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtComplaintAliceOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnRequest provides the Go interface for E_TableOtComplaintAliceOnRequest().
func (session *AliceSession) OnRequest(requestFile, responseFile string) error {
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

	ret := bool(C.E_TableOtComplaintAliceOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtComplaintAliceOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_TableOtComplaintAliceOnReceipt()
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

	ret := bool(C.E_TableOtComplaintAliceOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtComplaintAliceOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBobSession provides the Go interface for E_TableOtComplaintBobNew()
func NewBobSession(
	bulletinFile, publicPath string,
	sellerID, buyerID [40]uint8, demands, phantoms []types.Range,
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
		C.E_TableOtComplaintBobNew(
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
			"E_TableOtComplaintBobNew(%v, %v, %v, %v, %d, %v, %d) failed",
			handle, buyerID, sellerID,
			demands, nrDemands, phantoms, nrPhantoms)
	}

	return &BobSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_TableOtComplaintBobFree()
func (session *BobSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableOtComplaintBobFree(handle))
	if !ret {
		return fmt.Errorf("E_TableOtComplaintBobFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetNegoRequest provides the Go interface for E_TableOtComplaintBobGetNegoRequest()
func (session *BobSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtComplaintBobGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtComplaintBobGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_TableOtComplaintBobOnNegoResponse()
func (session *BobSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtComplaintBobOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtComplaintBobOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_TableOtComplaintBobOnNegoRequest()
func (session *BobSession) OnNegoRequest(requestFile, responseFile string) error {
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

	if ret := bool(C.E_TableOtComplaintBobOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtComplaintBobOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// GetRequest provides the Go interface for E_TableOtComplaintBobGetRequest()
func (session *BobSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtComplaintBobGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtComplaintBobGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_TableOtComplaintBobOnResponse()
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

	ret := bool(C.E_TableOtComplaintBobOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtComplaintBobOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_TableOtComplaintBobOnSecret()
func (session *BobSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_TableOtComplaintBobOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtComplaintBobOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_TableOtComplaintBobSaveDecrypted()
func (session *BobSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_TableOtComplaintBobSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtComplaintBobSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}

// GenerateClaim provides the Go interface for E_TableOtComplaintBobGenerateClaim()
func (session *BobSession) GenerateClaim(claimFile string) error {
	if err := utils.CheckDirOfPathExistence(claimFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	claimFileCStr := C.CString(claimFile)
	defer C.free(unsafe.Pointer(claimFileCStr))

	ret := bool(C.E_TableOtComplaintBobGenerateClaim(handle, claimFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtComplaintBobGenerateClaim(%v, %s) failed",
			handle, claimFile)
	}

	return nil
}
