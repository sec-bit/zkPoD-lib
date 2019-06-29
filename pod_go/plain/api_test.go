package plain

import (
	"testing"

	"github.com/sec-bit/zkPoD-lib/pod_go/setup"
)

const (
	testEccPubFile = "../test_data"

	testPlainPublishPathSucc = "../test_data/publish/plain"
	testPlainPublishPathFail = "../test_data/publish/plain_not_exist_dir"

	testPlainPublicPathSucc = "../test_data/publish/plain/public"
	testPlainPublicPathFail = "../test_data/publish/plain_not_exist_dir/public"

	testPlainBulletinSucc = "../test_data/publish/plain/bulletin"
	testPlainBulletinFail = "../test_data/publish/plain_not_exist_dir/bulletin"
)

var (
	inited = false
)

func prepare(t *testing.T) {
	if !inited {

		if ret := setup.Load(testEccPubFile); !ret {
			t.Fatalf("failed to load %s\n", testEccPubFile)
		}
		inited = true
	}
}

func TestNewA(t *testing.T) {
	prepare(t)

	if _, err := NewA(testPlainPublishPathSucc); err != nil {
		t.Errorf("%v\n", err)
	}

	if _, err := NewA(testPlainPublishPathFail); err == nil {
		t.Errorf("NewA(%s) should fail\n", testPlainPublishPathFail)
	}
}

func TestNewB(t *testing.T) {
	prepare(t)

	if _, err := NewB(testPlainBulletinSucc, testPlainPublicPathSucc); err != nil {
		t.Errorf("%v\n", err)
	}

	if _, err := NewB(testPlainBulletinFail, testPlainPublicPathFail); err == nil {
		t.Errorf("NewB(%s, %s) should fail\n",
			testPlainBulletinFail, testPlainPublicPathFail)
	}
}

func TestFreeA(t *testing.T) {
	prepare(t)

	a, err := NewA(testPlainPublishPathSucc)
	if err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := a.Free(); err != nil {
		t.Errorf("%v\n", err)
	}
}

func TestFreeB(t *testing.T) {
	prepare(t)

	b, err := NewB(testPlainBulletinSucc, testPlainPublicPathSucc)
	if err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := b.Free(); err != nil {
		t.Errorf("%v\n", err)
	}
}
