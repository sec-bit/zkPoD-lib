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

// NewAliceSession provides the Go interface for E_PlainOtComplaintAliceNew().
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
		C.E_PlainOtComplaintAliceNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_PlainOtComplaintAliceNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &AliceSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_PlainOtComplaintAliceFree()
func (session *AliceSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainOtComplaintAliceFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainOtComplaintAliceFree(%v) failed", handle)
	}
	return session.a.Free()
}

// GetNegoRequest provides the Go interface for E_PlainOtComplaintAliceGetNegoRequest()
func (session *AliceSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_PlainOtComplaintAliceGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_PlainOtComplaintAliceGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_PlainOtComplaintAliceOnNegoRequest()
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

	if ret := bool(C.E_PlainOtComplaintAliceOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintAliceOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_PlainOtComplaintAliceOnNegoResponse()
func (session *AliceSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_PlainOtComplaintAliceOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintAliceOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnRequest provides the Go interface for E_PlainOtComplaintAliceOnRequest().
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

	ret := bool(C.E_PlainOtComplaintAliceOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintAliceOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_PlainOtComplaintAliceOnReceipt()
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

	ret := bool(C.E_PlainOtComplaintAliceOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintAliceOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBobSession provides the Go interface for E_PlainOtComplaintBobNew()
func NewBobSession(
	bulletinFile, publicPath string,
	sellerID, buyerID [40]uint8, demands, phantoms []types.Range,
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
		C.E_PlainOtComplaintBobNew(
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
			"E_PlainOtComplaintBobNew(%v, %v, %v, %v, %d, %v, %d) failed",
			handle, buyerID, sellerID,
			demands, nrDemands, phantoms, nrPhantoms)
	}

	return &BobSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_PlainOtComplaintBobFree()
func (session *BobSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_PlainOtComplaintBobFree(handle))
	if !ret {
		return fmt.Errorf("E_PlainOtComplaintBobFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetNegoRequest provides the Go interface for E_PlainOtComplaintBobGetNegoRequest()
func (session *BobSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_PlainOtComplaintBobGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_PlainOtComplaintBobGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_PlainOtComplaintBobOnNegoResponse()
func (session *BobSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_PlainOtComplaintBobOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintBobOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_PlainOtComplaintBobOnNegoRequest()
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

	if ret := bool(C.E_PlainOtComplaintBobOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintBobOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// GetRequest provides the Go interface for E_PlainOtComplaintBobGetRequest()
func (session *BobSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_PlainOtComplaintBobGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_PlainOtComplaintBobGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_PlainOtComplaintBobOnResponse()
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

	ret := bool(C.E_PlainOtComplaintBobOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintBobOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_PlainOtComplaintBobOnSecret()
func (session *BobSession) OnSecret(secretFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	ret := bool(C.E_PlainOtComplaintBobOnSecret(handle, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintBobOnSecret(%v, %s) failed",
			handle, secretFile)
	}

	return nil
}

// Decrypt provides the Go interface for E_PlainOtComplaintBobSaveDecrypted()
func (session *BobSession) Decrypt(outFile string) error {
	if err := utils.CheckDirOfPathExistence(outFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	outFileCStr := C.CString(outFile)
	defer C.free(unsafe.Pointer(outFileCStr))

	ret := bool(C.E_PlainOtComplaintBobSaveDecrypted(
		handle, outFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintBobSaveDecrypted(%v, %s) failed",
			handle, outFile)
	}

	return nil
}

// GenerateClaim provides the Go interface for E_PlainOtComplaintBobGenerateClaim()
func (session *BobSession) GenerateClaim(claimFile string) error {
	if err := utils.CheckDirOfPathExistence(claimFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	claimFileCStr := C.CString(claimFile)
	defer C.free(unsafe.Pointer(claimFileCStr))

	ret := bool(C.E_PlainOtComplaintBobGenerateClaim(handle, claimFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_PlainOtComplaintBobGenerateClaim(%v, %s) failed",
			handle, claimFile)
	}

	return nil
}
