package ecc

import (
	"testing"
)

const (
	testEccPubFileSucc = "../test_data/ecc_pub.bin"
	testEccPubFileFail = "../test_data/ecc_pub_not_exist.bin"
)

func TestInit(t *testing.T) {
	Init()
}

func TestLoad(t *testing.T) {
	if ret := Load(testEccPubFileFail); ret != false {
		t.Errorf("loading %s should fail\n", testEccPubFileFail)
	}

	if ret := Load(testEccPubFileSucc); ret != true {
		t.Errorf("failed to load %s\n", testEccPubFileSucc)
	}
}
