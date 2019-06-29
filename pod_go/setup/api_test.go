package setup

import (
	"testing"
)

const (
	testBinPubPathSucc = "../test_data"
	testBinPubPathFail = "../test_data_not_exist"
)

func TestLoad(t *testing.T) {
	if ret := Load(testBinPubPathFail); ret != false {
		t.Errorf("loading %s should fail\n", testBinPubPathFail)
	}

	if ret := Load(testBinPubPathSucc); ret != true {
		t.Errorf("failed to load %s\n", testBinPubPathSucc)
	}
}
