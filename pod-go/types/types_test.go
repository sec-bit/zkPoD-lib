package types

import (
	"testing"
)

var (
	u8Array = []uint8{0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xdc, 0xfe}
)

func prepareUInt8(t *testing.T) *CUInt8 {
	cvals := NewUInt8(u8Array)
	if cvals == nil {
		t.Fatalf("NewUInt8(%v) failed", u8Array)
	}
	return cvals
}

func TestNewUInt8(t *testing.T) {
	prepareUInt8(t)
}

func TestCUInt8Slice(t *testing.T) {
	cvals := prepareUInt8(t)
	vals := cvals.Slice(len(u8Array))

	for i, v := range vals {
		if v != u8Array[i] {
			t.Fatalf("cvals[%d] = %d, %d expected",
				i, v, u8Array[i])
		}
	}
}

func TestCUInt8Free(t *testing.T) {
	cvals := prepareUInt8(t)
	cvals.Free()
}
