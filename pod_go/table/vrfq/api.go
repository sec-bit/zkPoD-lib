package vrfq

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

// NewAliceSession provides the Go interface for E_TableVrfqAliceNew().
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
		C.E_TableVrfqAliceNew(
			handle,
			(*C.uint8_t)(sellerIDCPtr),
			(*C.uint8_t)(buyerIDCPtr)))
	if session == nil {
		a.Free()
		return nil, fmt.Errorf(
			"E_TableVrfqAliceNew(%v, %v, %v) failed",
			handle, sellerID, buyerID)
	}

	return &AliceSession{a: a, handle: session}, nil
}

// Free provides the Go interface for E_TableVrfqAliceFree()
func (session *AliceSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableVrfqAliceFree(handle))
	if !ret {
		return fmt.Errorf("E_TableVrfqAliceFree(%v) failed", handle)
	}
	return session.a.Free()
}

// OnRequest provides the Go interface for E_TableVrfqAliceOnRequest().
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

	ret := bool(C.E_TableVrfqAliceOnRequest(
		handle, requestFileCStr, responseFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableVrfqAliceOnRequest(%v, %s, %s) failed",
			handle, requestFile, responseFile)
	}

	return nil
}

// OnReceipt provides the Go interface for E_TableVrfqAliceOnReceipt()
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

	ret := bool(C.E_TableVrfqAliceOnReceipt(
		handle, receiptFileCStr, secretFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableVrfqAliceOnReceipt(%v, %s, %s) failed",
			handle, receiptFile, secretFile)
	}

	return nil
}

// NewBobSession provides the Go interface for E_TableVrfqBobNew()
func NewBobSession(
	bulletinFile, publicPath string, sellerID, buyerID [40]uint8,
	keyName string, keyValues []string,
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

	nrVals := len(keyValues)

	cVals := make([](*C.char), nrVals)
	for idx, val := range keyValues {
		cVals[idx] = C.CString(val)
	}
	defer func() {
		for _, cVal := range cVals {
			C.free(unsafe.Pointer(cVal))
		}
	}()
	cValsPtr := (**C.char)(unsafe.Pointer(&cVals[0]))

	handle := C.handle_t(b.CHandle())
	session := types.CHandle(
		C.E_TableVrfqBobNew(
			handle,
			(*C.uint8_t)(buyerIDCPtr),
			(*C.uint8_t)(sellerIDCPtr),
			keyNameCStr,
			cValsPtr,
			C.uint64_t(nrVals)))
	if session == nil {
		b.Free()
		return nil, fmt.Errorf(
			"E_TableVrfqBobNew(%v, %v, %v, %v, %v, %v) failed",
			handle, buyerID, sellerID, keyName, keyValues, nrVals)
	}

	return &BobSession{b: b, handle: session}, nil
}

// Free provides the Go interface for E_TableVrfqBobFree()
func (session *BobSession) Free() error {
	handle := C.handle_t(session.handle)
	ret := bool(C.E_TableVrfqBobFree(handle))
	if !ret {
		return fmt.Errorf("E_TableVrfqBobFree(%v) failed", handle)
	}
	return session.b.Free()
}

// GetRequest provides the Go interface for E_TableVrfqBobGetRequest()
func (session *BobSession) GetRequest(requestFile string) error {
	if err := utils.CheckDirOfPathExistence(requestFile); err != nil {
		return err
	}

	handle := C.handle_t(session.handle)

	requestFileCStr := C.CString(requestFile)
	defer C.free(unsafe.Pointer(requestFileCStr))

	if ret := bool(
		C.E_TableVrfqBobGetRequest(handle, requestFileCStr)); !ret {
		return fmt.Errorf("E_TableVrfqBobGetRequest(%v, %s) failed",
			handle, requestFile)
	}

	return nil
}

// OnResponse provides the Go interface for E_TableVrfqBobOnResponse()
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

	ret := bool(C.E_TableVrfqBobOnResponse(
		handle, responseFileCStr, receiptFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableVrfqBobOnResponse(%v, %s, %s) failed",
			handle, responseFile, receiptFile)
	}

	return nil
}

// OnSecret provides the Go interface for E_TableVrfqBobOnSecret()
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

	ret := bool(C.E_TableVrfqBobOnSecret(
		handle, secretFileCStr, positionsFileCStr))
	if !ret {
		return fmt.Errorf(
			"E_TableVrfqBobOnSecret(%v, %s, %s) failed",
			handle, secretFile, positionsFile)
	}

	return nil
}
