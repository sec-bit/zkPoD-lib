ifeq ($(OS),Windows_NT)
	OS_TYPE := Windows
else
	UNAME := $(shell uname -s)
	ifeq ($(UNAME),Linux)
		OS_TYPE := Linux
	else ifeq ($(UNAME),Darwin)
		OS_TYPE := Darwin
	else
		OS_TYPE := Unknown
	endif
endif

GO   ?= $(shell which go)
MAKE ?= $(shell which make)

ifndef V
  V := @
else
  V :=
  GO_TEST_FLAGS := -v
endif

POD_CORE_DIR := $(PWD)/../pod_core
POD_SETUP_DIR := $(PWD)/../pod_setup
POD_PUBLISH_DIR := $(PWD)/../pod_publish

POD_SETUP := $(POD_SETUP_DIR)/pod_setup
POD_PUBLISH := $(POD_PUBLISH_DIR)/pod_publish
ifeq ($(OS_TYPE),Windows)
  POD_SETUP := $(addsuffix .exe,$(POD_SETUP))
  POD_PUBLISH := $(addsuffix .exe,$(POD_PUBLISH))
endif

# -a option of pod_setup
POD_SETUP_OPTION_A := 1026
# -c option of pod_publish in plain mode
POD_PUBLISH_OPTION_C := 1023

CGO_LDFLAGS := -L$(POD_CORE_DIR)

TEST_DATA_DIR := $(PWD)/test_data

TEST_ECC_PUB := $(TEST_DATA_DIR)/ecc_pub.bin
TEST_PLAIN_FILE := $(TEST_DATA_DIR)/plain.txt
TEST_TABLE_FILE := $(TEST_DATA_DIR)/table.csv

TEST_OUTPUT_DIR := $(TEST_DATA_DIR)/output

TEST_PUBLISH_DIR := $(TEST_DATA_DIR)/publish
TEST_PUBLISH_PLAIN_DIR := $(TEST_PUBLISH_DIR)/plain
TEST_PUBLISH_TABLE_DIR := $(TEST_PUBLISH_DIR)/table

PKG_ROOT_NAME := github.com/sec-bit/zkPoD-lib/pod_go

GO_TEST := LD_LIBRARY_PATH=$(POD_CORE_DIR) \
		CGO_LDFLAGS=$(CGO_LDFLAGS) \
		$(GO) test $(GO_TEST_FLAGS)

all:

test: test_publish pod_core
	@echo "+ Testing ..."
	$(V) rm -rf $(TEST_OUTPUT_DIR)
	$(V) mkdir -p $(TEST_OUTPUT_DIR)
	$(V) $(GO) clean -testcache ./...
	$(V) $(GO_TEST) ./...

test_publish: test_publish_plain test_publish_table

test_publish_plain: $(POD_PUBLISH) $(TEST_ECC_PUB) $(TEST_PLAIN_FILE)
	@echo "+ Publishing " $(TEST_PLAIN_FILE) "..."
	$(V) rm -rf $(TEST_PUBLISH_PLAIN_DIR)
	$(V) mkdir -p $(TEST_PUBLISH_PLAIN_DIR)
	$(V) $(POD_PUBLISH) \
		-e $(TEST_ECC_PUB) \
		-m plain \
		-f $(TEST_PLAIN_FILE) \
		-o $(TEST_PUBLISH_PLAIN_DIR) \
		-c $(POD_PUBLISH_OPTION_C)

test_publish_table: $(POD_PUBLISH) $(TEST_ECC_PUB) $(TEST_TABLE_FILE)
	@echo "+ Publishing " $(TEST_TABLE_FILE) "..."
	$(V) rm -rf $(TEST_PUBLISH_TABLE_DIR)
	$(V) mkdir -p $(TEST_PUBLISH_TABLE_DIR)
	$(V) $(POD_PUBLISH) \
		-e $(TEST_ECC_PUB) \
		-m table \
		-f $(TEST_TABLE_FILE) \
		-o $(TEST_PUBLISH_TABLE_DIR) \
		-t csv \
		-k 0 1 2 3 4 5

pod_core:
	@echo "+ Building pod_core ..."
	$(V) cd $(POD_CORE_DIR); $(MAKE) lib

$(POD_SETUP):
	@echo "+ Building pod_setup ..."
	$(V) cd $(POD_SETUP_DIR); $(MAKE)

$(POD_PUBLISH):
	@echo "+ Building pod_publish ..."
	$(V) cd $(POD_PUBLISH_DIR); $(MAKE)

$(TEST_ECC_PUB): $(POD_SETUP)
	@echo "+ Generating ecc_pub"
	$(V) $(POD_SETUP) -o $(TEST_ECC_PUB) -a $(POD_SETUP_OPTION_A)

clean:
	$(V) rm -f $(TEST_ECC_PUB)
	$(V) rm -rf $(TEST_PUBLISH_DIR)
	$(V) rm -rf $(TEST_OUTPUT_DIR)