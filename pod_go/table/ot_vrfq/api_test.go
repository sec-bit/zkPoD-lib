package ot_vrfq

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

	testOutputDir = "../../test_data/output/table_otvrfq"

	testIncorrectSecret = "../../test_data/table_otvrfq_incorrect_secret"
)

var (
	inited = false

	sellerID = [40]uint8{
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
	demandVals := []string{
		"20190417",
		"20190415",
		"20180105",
		"20170818",
	}
	phantomVals := []string{
		"20190417",
		"20190415",
		"20180105",
		"20170818",

		"20190411",
		"20190407",
		"20190331",
		"20181101",
		"20181026",
		"20181022",
		"20181018",
		"20181010",
	}

	session, err := NewBobSession(
		testBulletin, testPublicPath,
		sellerID, buyerID, "date (UTC)", demandVals, phantomVals)
	if err != nil {
		t.Fatalf("%v\n", err)
	}

	return session
}

func prepareSessions(t *testing.T, dir string) (*AliceSession, *BobSession) {
	seller := prepareAliceSession(t)
	buyer := prepareBobSession(t)

	sellerReqFile := fmt.Sprintf("%s/seller_nego_request", dir)
	sellerRespFile := fmt.Sprintf("%s/seller_nego_response", dir)
	buyerReqFile := fmt.Sprintf("%s/buyer_nego_request", dir)
	buyerRespFile := fmt.Sprintf("%s/buyer_nego_response", dir)

	if err := buyer.GetNegoRequest(buyerReqFile); err != nil {
		t.Fatal(err)
	}
	if err := seller.OnNegoRequest(buyerReqFile, sellerRespFile); err != nil {
		t.Fatal(err)
	}
	if err := buyer.OnNegoResponse(sellerRespFile); err != nil {
		t.Fatal(err)
	}

	if err := seller.GetNegoRequest(sellerReqFile); err != nil {
		t.Fatal(err)
	}
	if err := buyer.OnNegoRequest(sellerReqFile, buyerRespFile); err != nil {
		t.Fatal(err)
	}
	if err := seller.OnNegoResponse(buyerRespFile); err != nil {
		t.Fatal(err)
	}

	return seller, buyer
}

func TestNewAliceSession(t *testing.T) {
	prepare(t)

	if _, err := NewAliceSession(testPublishPath, sellerID, buyerID); err != nil {
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

	demandVals := []string{
		"20190417",
		"20190415",
		"20180105",
		"20170818",
	}
	phantomVals := []string{
		"20190417",
		"20190415",
		"20180105",
		"20170818",

		"20190411",
		"20190407",
		"20190331",
		"20181101",
		"20181026",
		"20181022",
		"20181018",
		"20181010",
	}

	if _, err := NewBobSession(
		testBulletin, testPublicPath,
		sellerID, buyerID, "date (UTC)", demandVals, phantomVals,
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

func TestBuyerGetNegoRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/BuyerGetNegoRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/buyer_nego_request", dir)

	prepare(t)

	session := prepareBobSession(t)
	defer session.Free()

	if err := session.GetNegoRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestSellerGetNegoRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/SellerGetNegoRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/seller_nego_request", dir)

	prepare(t)

	session := prepareAliceSession(t)
	defer session.Free()

	if err := session.GetNegoRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestSellerOnNegoRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/SellerOnNegoRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/buyer_nego_request", dir)
	responseFile := fmt.Sprintf("%s/seller_nego_response", dir)

	prepare(t)
	seller := prepareAliceSession(t)
	defer seller.Free()
	buyer := prepareBobSession(t)
	defer buyer.Free()

	if err := buyer.GetNegoRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnNegoRequest(requestFile, responseFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestBuyerOnNegoRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/BuyerOnNegoRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/seller_nego_request", dir)
	responseFile := fmt.Sprintf("%s/buyer_nego_response", dir)

	prepare(t)
	seller := prepareAliceSession(t)
	defer seller.Free()
	buyer := prepareBobSession(t)
	defer buyer.Free()

	if err := seller.GetNegoRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnNegoRequest(requestFile, responseFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestBuyerOnNegoResponse(t *testing.T) {
	dir := fmt.Sprintf("%s/BuyerOnNegoResponse", testOutputDir)
	mkdir(t, dir)
	buyerReqFile := fmt.Sprintf("%s/buyer_nego_request", dir)
	sellerRespFile := fmt.Sprintf("%s/seller_nego_response", dir)

	prepare(t)
	seller := prepareAliceSession(t)
	defer seller.Free()
	buyer := prepareBobSession(t)
	defer buyer.Free()

	if err := buyer.GetNegoRequest(buyerReqFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnNegoRequest(buyerReqFile, sellerRespFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnNegoResponse(sellerRespFile); err != nil {
		t.Fatal(err)
	}
}

func TestSellerOnNegoResponse(t *testing.T) {
	dir := fmt.Sprintf("%s/SellerOnNegoResponse", testOutputDir)
	mkdir(t, dir)
	sellerReqFile := fmt.Sprintf("%s/seller_nego_request", dir)
	buyerRespFile := fmt.Sprintf("%s/buyer_nego_response", dir)

	prepare(t)
	seller := prepareAliceSession(t)
	defer seller.Free()
	buyer := prepareBobSession(t)
	defer buyer.Free()

	if err := seller.GetNegoRequest(sellerReqFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := buyer.OnNegoRequest(sellerReqFile, buyerRespFile); err != nil {
		t.Fatalf("%v\n", err)
	}

	if err := seller.OnNegoResponse(buyerRespFile); err != nil {
		t.Fatal(err)
	}
}

func TestGetRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/GetRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)

	prepare(t)
	seller, buyer := prepareSessions(t, dir)
	defer func() {
		seller.Free()
		buyer.Free()
	}()

	if err := buyer.GetRequest(requestFile); err != nil {
		t.Fatalf("%v\n", err)
	}
}

func TestOnRequest(t *testing.T) {
	dir := fmt.Sprintf("%s/OnRequest", testOutputDir)
	mkdir(t, dir)
	requestFile := fmt.Sprintf("%s/request", dir)
	responseFile := fmt.Sprintf("%s/response", dir)

	prepare(t)
	seller, buyer := prepareSessions(t, dir)
	defer func() {
		seller.Free()
		buyer.Free()
	}()

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
	seller, buyer := prepareSessions(t, dir)
	defer func() {
		seller.Free()
		buyer.Free()
	}()

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
	seller, buyer := prepareSessions(t, dir)
	defer func() {
		seller.Free()
		buyer.Free()
	}()

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
	seller, buyer := prepareSessions(t, dir)
	defer func() {
		seller.Free()
		buyer.Free()
	}()

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
	seller, buyer := prepareSessions(t, dir)
	defer func() {
		seller.Free()
		buyer.Free()
	}()

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
