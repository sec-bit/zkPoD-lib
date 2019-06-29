package ot_vrfq

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

// NewAliceSession provides the Go interface for E_TableOtVrfqAliceNew().
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
		C.E_TableOtVrfqAliceNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_TableOtVrfqAliceNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &AliceSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_TableOtVrfqAliceFree()
func (session *AliceSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableOtVrfqAliceFree(handle))
	if !ret {
		return fmt.Errorf("E_TableOtVrfqAliceFree(%v) failed", handle)
	}
	return session.a.Free()
}

// GetNegoRequest provides the Go interface for E_TableOtVrfqAliceGetNegoRequest()
func (session *AliceSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtVrfqAliceGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtVrfqAliceGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_TableOtVrfqAliceOnNegoRequest()
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

	if ret := bool(C.E_TableOtVrfqAliceOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtVrfqAliceOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_TableOtVrfqAliceOnNegoResponse()
func (session *AliceSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtVrfqAliceOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtVrfqAliceOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnRequest provides the Go interface for E_TableOtVrfqAliceOnRequest().
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

	ret := bool(C.E_TableOtVrfqAliceOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtVrfqAliceOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_TableOtVrfqAliceOnReceipt()
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

	ret := bool(C.E_TableOtVrfqAliceOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtVrfqAliceOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBobSession provides the Go interface for E_TableOtVrfqBobNew()
func NewBobSession(
	bulletinFile, publicPath string, sellerID, buyerID [40]uint8,
	keyName string, demandKeyValues, phantomKeyValues []string,
) (*BobSession, error) {
	b, err := table.NewB(bulletinFile, publicPath)
	if err != nil {
		return nil, err
	}

	sellerIDCPtr := types.NewUInt8(sellerID[:])
	defer sellerIDCPtr.Free()

	buyerIDCPtr := types.NewUInt8(buyerID[:])
	defer buyerIDCPtr.Free()

	keyNameCStr := C.CString(keyName)
	defer C.free(unsafe.Pointer(keyNameCStr))

	nrDemandVals := len(demandKeyValues)
	nrPhantomVals := len(phantomKeyValues)

	cDemandVals := make([](*C.char), nrDemandVals)
	for idx, val := range demandKeyValues {
		cDemandVals[idx] = C.CString(val)
	}
	defer func() {
		for _, cVal := range cDemandVals {
			C.free(unsafe.Pointer(cVal))
		}
	}()
	cDemandValsPtr := (**C.char)(unsafe.Pointer(&cDemandVals[0]))

	cPhantomVals := make([](*C.char), nrPhantomVals)
	for idx, val := range phantomKeyValues {
		cPhantomVals[idx] = C.CString(val)
	}
	defer func() {
		for _, cVal := range cPhantomVals {
			C.free(unsafe.Pointer(cVal))
		}
	}()
	cPhantomValsPtr := (**C.char)(unsafe.Pointer(&cPhantomVals[0]))

	handle := C.handle_t(b.CHandle())
	session := types.CHandle(
		C.E_TableOtVrfqBobNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			keyNameCStr,
			cDemandValsPtr, C.uint64_t(nrDemandVals),
			cPhantomValsPtr, C.uint64_t(nrPhantomVals)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_TableOtVrfqBobNew(%v, %v, %v, %v, %v, %v, %v, %v) failed",
			handle, buyerID, sellerID, keyName,
			demandKeyValues, nrDemandVals,
			phantomKeyValues, nrPhantomVals)
	}

	return &BobSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_TableOtVrfqBobFree()
func (session *BobSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableOtVrfqBobFree(handle))
	if !ret {
		return fmt.Errorf("E_TableOtVrfqBobFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetNegoRequest provides the Go interface for E_TableOtVrfqBobGetNegoRequest()
func (session *BobSession) GetNegoRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtVrfqBobGetNegoRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtVrfqBobGetNegoRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnNegoResponse provides the Go interface for E_TableOtVrfqBobOnNegoResponse()
func (session *BobSession) OnNegoResponse(responseFile string) error {
	if err := utils.CheckRegularFileReadPerm(responseFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	responseFileCStr := C.CString(responseFile)
	defer C.free(unsafe.Pointer(responseFileCStr))

	if ret := bool(C.E_TableOtVrfqBobOnNegoResponse(
		handle, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtVrfqBobOnNegoResponse(%v, %s) failed",
			handle, responseFile)
	}

	return nil
}

// OnNegoRequest provides the Go interface for E_TableOtVrfqBobOnNegoRequest()
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

	if ret := bool(C.E_TableOtVrfqBobOnNegoRequest(
		handle, requestFileCStr, responseFileCStr)); !ret {
		return fmt.Errorf(
			"E_TableOtVrfqBobOnNegoRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// GetRequest provides the Go interface for E_TableOtVrfqBobGetRequest()
func (session *BobSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableOtVrfqBobGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableOtVrfqBobGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_TableOtVrfqBobOnResponse()
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

	ret := bool(C.E_TableOtVrfqBobOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtVrfqBobOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_TableOtVrfqBobOnSecret()
func (session *BobSession) OnSecret(secretFile, positionsFile string) error {
	if err := utils.CheckRegularFileReadPerm(secretFile); err != nil {
		return err
	}
	if err := utils.CheckDirOfPathExistence(positionsFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	secretFileCStr := C.CString(secretFile)
	defer C.free(unsafe.Pointer(secretFileCStr))

	positionsFileCStr := C.CString(positionsFile)
	defer C.free(unsafe.Pointer(positionsFileCStr))

	ret := bool(C.E_TableOtVrfqBobOnSecret(
		handle, secretFileCStr, positionsFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableOtVrfqBobOnSecret(%v, %s, %s) failed",
			handle, secretFile, positionsFile)
	}

	return nil
}
