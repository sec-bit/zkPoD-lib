package table

import (
	"testing"

	"github.com/sec-bit/zkPoD-lib/pod_go/setup"
)

const (
	testEccPubFile = "../test_data"

	testPublishPathSucc = "../test_data/publish/table"
	testPublishPathFail = "../test_data/publish/table_not_exist_dir"

	testPublicPathSucc = "../test_data/publish/table/public"
	testPublicPathFail = "../test_data/publish/table_not_exist_dir/public"

	testBulletinSucc = "../test_data/publish/table/bulletin"
	testBulletinFail = "../test_data/publish/table_not_exist_dir/bulletin"
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

	if _, err := NewA(testPublishPathSucc); err != nil {
		t.Errorf("%v\n", err)
	}

	if _, err := NewA(testPublishPathFail); err == nil {
		t.Errorf("NewA(%s) should fail\n", testPublishPathFail)
	}
}

func TestNewB(t *testing.T) {
	prepare(t)

	if _, err := NewB(testBulletinSucc, testPublicPathSucc); err != nil {
		t.Errorf("%v\n", err)
	}

	if _, err := NewB(testBulletinFail, testPublicPathFail); err == nil {
		t.Errorf("NewB(%s, %s) should fail\n",
			testBulletinFail, testPublicPathFail)
	}
}

func TestFreeA(t *testing.T) {
	prepare(t)

	a, err := NewA(testPublishPathSucc)
	if err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := a.Free(); err != nil {
		t.Errorf("%v\n", err)
	}
}

func TestFreeB(t *testing.T) {
	prepare(t)

	b, err := NewB(testBulletinSucc, testPublicPathSucc)
	if err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := b.Free(); err != nil {
		t.Errorf("%v\n", err)
	}
}
