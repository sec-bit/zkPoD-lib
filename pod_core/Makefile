#sudo apt-get install libomp-dev
#sudo apt-get install libcrypto++-dev
#sudo apt-get install libboost-all-dev

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

CXX ?= $(shell which g++)
ifdef CXX11_HOME
  CXX := LD_LIBRARY_PATH=${CXX11_HOME}/lib $(CXX11_HOME)/bin/g++
endif

# HOME = ../linux
THIRD_PARTY_HOME = ../thirdparty
YAS_HOME = ../thirdparty/yas/include
PUB_HOME = ../public
SNARK_HOME = ../depends/install
MCL_HOME = $(SNARK_HOME)/include/depends/mcl/include

INC_FLAGS :=  \
 -I /usr/local/include \
 -I /usr/include \
 -I $(MCL_HOME) \
 -I $(PUB_HOME) \
 -I $(YAS_HOME) \
 -I $(THIRD_PARTY_HOME) \
 -isystem $(SNARK_HOME)/include \
 -I. \
 -I.. \

ifdef CXX11_HOME
  INC_FLAGS += -I $(CXX11_HOME)/include
endif

CXXFLAGS := \
 -g3 \
 -fPIC \
 -fopenmp \
 -std=c++17 \
 -Wall \
 -Wextra \
 -gdwarf-2 \
 -gstrict-dwarf \
 -Wno-parentheses \
 -Wdeprecated-declarations \
 -fmerge-all-constants  \
 $(INC_FLAGS) \

SNARK_FLAGS = \
 -DBINARY_OUTPUT \
 -DBN_SUPPORT_SNARK=1 \
 -DCURVE_MCL_BN128 \
 -DMONTGOMERY_OUTPUT \
 -DMULTICORE=1 \
 -DUSE_ASM \
 -DNO_PROCPS \

MCL_FLAGS := \
 -DMCL_DONT_USE_OPENSSL

BOOST_FLAGS := \
 -DBOOST_SPIRIT_THREADSAFE 

RELEASE_FLAGS := \
 -O3 \
 -DNDEBUG \
 -DLINUX \

DEBUG_FLAGS := \
 -O0 \
 -D_DEBUG \
 -DLINUX \
 
#LDFLAGS += \
# -static-libstdc++ -static-libgcc \
# -fmerge-all-constants \
# -L${CXX11_HOME}/lib64

LDFLAGS += \
 -fmerge-all-constants \
 -L$(SNARK_HOME)/lib

LIBS := \
 -lpthread \
 -lrt \
 -lgmpxx \
 -lgmp \
 -lcryptopp \
 -lgomp \
 -lboost_filesystem \
 -lboost_system \
 -lboost_iostreams \
 -lboost_program_options \
 -lsnark \
 -lff \
 -lmcl \

ifeq ($(OS_TYPE),Darwin)
  TMPVAR := $(CXXFLAGS)
  CXXFLAGS := $(filter-out -fopenmp, $(TMPVAR))

  TMPVAR := $(SNARK_FLAGS)
  SNARK_FLAGS := $(filter-out -DMULTICORE=1, $(TMPVAR))

  TMPVAR := $(SNARK_FLAGS)
  SNARK_FLAGS := $(filter-out -DUSE_ASM, $(TMPVAR))

  TMPVAR := $(LIBS)
  TMPVAR := $(filter-out -lgomp, $(TMPVAR))
  LIBS := $(filter-out -lrt, $(TMPVAR))
endif

BUILD := ./build/self
BUILDPUB := ./build/public
BUILDCAPI := ./build/self/capi
BUILD3RD := ./build/thirdparty

# $(shell mkdir -p $(HOME)/bin)
$(shell mkdir -p $(BUILD))
$(shell mkdir -p $(BUILDPUB))
$(shell mkdir -p $(BUILDCAPI))
$(shell mkdir -p $(BUILD3RD))

DIR := . ../public capi

SRC := $(foreach d, $(DIR), $(wildcard $(d)/*.cc))

RELEASE_OBJ := $(patsubst %.cc, $(BUILD)/%.o, $(SRC))

DEBUG_OBJ := $(patsubst %.cc, $(BUILD)/%.d.o, $(SRC))

RELEASE_BIN := pod_core
DEBUG_BIN := pod_cored
ifeq ($(OS_TYPE),Windows)
  RELEASE_BIN = $(addsuffix .exe,$(RELEASE_BIN))
  DEBUG_BIN = $(addsuffix .exe,$(DEBUG_BIN))
endif

RELEASE_LIB := libpod_core
DEBUG_LIB := libpod_cored
ifeq ($(OS_TYPE),Windows)
  RELEASE_LIB := $(addsuffix .dll,$(RELEASE_LIB))
  DEBUG_LIB := $(addsuffix .dll,$(DEBUG_LIB))
else
  RELEASE_LIB := $(addsuffix .so,$(RELEASE_LIB))
  DEBUG_LIB := $(addsuffix .so,$(DEBUG_LIB))
endif

all: bin lib

$(BUILD)/%.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(RELEASE_FLAGS) $(MCL_FLAGS) $(SNARK_FLAGS) $(BOOST_FLAGS) -o $@

$(BUILD)/%.d.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(DEBUG_FLAGS) $(MCL_FLAGS)  $(SNARK_FLAGS) $(BOOST_FLAGS) -o $@

bin: $(RELEASE_BIN) $(DEBUG_BIN)

$(DEBUG_BIN) : $(DEBUG_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

$(RELEASE_BIN) : $(RELEASE_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

lib: $(RELEASE_LIB) $(DEBUG_LIB)

$(DEBUG_LIB) : $(DEBUG_OBJ)
	$(CXX) $^ -shared -o $@ $(LDFLAGS) $(LIBS)

$(RELEASE_LIB) : $(RELEASE_OBJ)
	$(CXX) $^ -shared -o $@ $(LDFLAGS) $(LIBS)

clean:
	find . -name "*.o" -delete
	rm -rf $(RELEASE_BIN) $(RELEASE_LIB) $(DEBUG_BIN) $(DEBUG_LIB)
	rm -rf build

.PHONY :
