package vrfq

import (
	"fmt"
	"os"
	"testing"

	"github.com/sec-bit/zkPoD-lib/pod_go/setup"
)

const (
	testEccPubFile  = "../../test_data"
	testPublishPath = "../../test_data/publish/table"
	testPublicPath  = "../../test_data/publish/table/public"
	testBulletin    = "../../test_data/publish/table/bulletin"

	testOutputDir = "../../test_data/output/table_vrfq"

	testIncorrectSecret = "../../test_data/table_vrfq_incorrect_secret"
)

var (
	inited = false

	sellerID = [40]uint8{
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
		0x12, 0x34, 0x56, 0x78, 0x9a, 0xbc, 0xde, 0xf0,
	}
	buyerID = [40]uint8{
		0x0f, 0xde, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21,
		0x0f, 0xde, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21,
		0x0f, 0xde, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21,
		0x0f, 0xde, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21,
		0x0f, 0xde, 0xcb, 0xa9, 0x87, 0x65, 0x43, 0x21,
	}
)

func mkdir(t *testing.T, path string) {
	if err := os.Mkdir(path, 0700); err != nil {
		t.Fatalf("failed to create directory %s: %v", path, err)
	}
}

func prepare(t *testing.T) {
	if inited {
		return
	}

	mkdir(t, testOutputDir)

	if ret := setup.Load(testEccPubFile); !ret {
		t.Fatalf("failed to load %s\n", testEccPubFile)
	}
	inited = true
}

func prepareAliceSession(t *testing.T) *AliceSession {
	session, err := NewAliceSession(testPublishPath, sellerID, buyerID)
	if err != nil {
		t.Fatalf("%v\n", err)
	}

	return session
}

func prepareBobSession(t *testing.T) *BobSession {
	vals := []string{
		"20190417",
		"20190415",
		"20180105",
		"20170818",
	}

	session, err := NewBobSession(
		testBulletin, testPublicPath, sellerID, buyerID, "date (UTC)", vals)
	if err != nil {
		t.Fatalf("%v\n", err)
	}

	return session
}

func TestNewAliceSession(t *testing.T) {
	prepare(t)

	if _, err := NewAliceSession(
		testPublishPath, sellerID, buyerID,
	); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestFreeAliceSession(t *testing.T) {
	prepare(t)

	session := prepareAliceSession(t)

	if err := session.Free(); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestNewBobSession(t *testing.T) {
	prepare(t)

	vals := []string{
		"20190417",
		"20190415",
		"20180105",
		"20170818",
	}

	if _, err := NewBobSession(
		testBulletin, testPublicPath, sellerID, buyerID, "date (UTC)", vals,
	); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestFreeBobSession(t *testing.T) {
	prepare(t)

	session := prepareBobSession(t)

	if err := session.Free(); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestGetRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/GetRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)

	prepare(t)

	session := prepareBobSession(t)
	defer session.Free()

	if err := session.GetRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestOnRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/OnRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)
	responseFile := fmt.Sprintf("%s/response", dir)

	prepare(t)

	buyer := prepareBobSession(t)
	defer buyer.Free()
	seller := prepareAliceSession(t)
	defer seller.Free()

	if err := buyer.GetRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnRequest(requestFile, responseFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestOnResponse(t *testing.T) {
	dir := fmt.Sprintf("%s/OnResponse", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)
	responseFile := fmt.Sprintf("%s/response", dir)
	receiptFile := fmt.Sprintf("%s/receipt", dir)

	prepare(t)

	buyer := prepareBobSession(t)
	defer buyer.Free()
	seller := prepareAliceSession(t)
	defer seller.Free()

	if err := buyer.GetRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnRequest(requestFile, responseFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnResponse(responseFile, receiptFile); err != nil {
		t.Fatalf("%v\n", err)
	}

}

func TestOnReceipt(t *testing.T) {
	dir := fmt.Sprintf("%s/OnReceipt", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)
	responseFile := fmt.Sprintf("%s/response", dir)
	receiptFile := fmt.Sprintf("%s/receipt", dir)
	secretFile := fmt.Sprintf("%s/secret", dir)

	prepare(t)

	buyer := prepareBobSession(t)
	defer buyer.Free()
	seller := prepareAliceSession(t)
	defer seller.Free()

	if err := buyer.GetRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnRequest(requestFile, responseFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnResponse(responseFile, receiptFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnReceipt(receiptFile, secretFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestOnSecret(t *testing.T) {
	dir := fmt.Sprintf("%s/OnSecret", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)
	responseFile := fmt.Sprintf("%s/response", dir)
	receiptFile := fmt.Sprintf("%s/receipt", dir)
	secretFile := fmt.Sprintf("%s/secret", dir)
	positionsFile := fmt.Sprintf("%s/positions", dir)

	prepare(t)

	buyer := prepareBobSession(t)
	defer buyer.Free()
	seller := prepareAliceSession(t)
	defer seller.Free()

	if err := buyer.GetRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnRequest(requestFile, responseFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnResponse(responseFile, receiptFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnReceipt(receiptFile, secretFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnSecret(secretFile, positionsFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestOnSecretFail(t *testing.T) {
	dir := fmt.Sprintf("%s/OnSecretFail", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)
	responseFile := fmt.Sprintf("%s/response", dir)
	receiptFile := fmt.Sprintf("%s/receipt", dir)
	secretFile := fmt.Sprintf("%s/secret", dir)
	positionsFile := fmt.Sprintf("%s/positions", dir)

	prepare(t)

	buyer := prepareBobSession(t)
	defer buyer.Free()
	seller := prepareAliceSession(t)
	defer seller.Free()

	if err := buyer.GetRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnRequest(requestFile, responseFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnResponse(responseFile, receiptFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnReceipt(receiptFile, secretFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnSecret(
		testIncorrectSecret, positionsFile,
	); err == nil {
		t.Fatalf("should fail: invalid secret %s\n",
			testIncorrectSecret)
	}
}
