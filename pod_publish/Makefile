#CXX11_HOME = /home/alpha/opt/gcc
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

THIRD_PARTY_HOME = ../thirdparty
MCL_HOME = ../thirdparty/mcl/include
CSV_HOME = ../thirdparty/csv
PUB_HOME = ../public

INC_FLAGS :=  \
 -I $(MCL_HOME) \
 -I $(PUB_HOME) \
 -I $(THIRD_PARTY_HOME) \
 -isystem $(CSV_HOME) \
 -I.

ifdef CXX11_HOME
  INC_FLAGS += -I $(CXX11_HOME)/include
endif

LDFLAGS += \
 -static-libstdc++ -static-libgcc \
 -fmerge-all-constants \

ifdef CXX11_HOME
  LDFLAGS += -L${CXX11_HOME}/lib64
endif

CXXFLAGS := \
 -g3 \
 -fPIC \
 -fopenmp \
 -std=gnu++17 \
 -Wall \
 -Wextra \
 -gdwarf-2 \
 -gstrict-dwarf \
 -Wno-parentheses \
 -Wdeprecated-declarations \
 -fmerge-all-constants  \
 $(INC_FLAGS)

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
 -L${CXX11_HOME}/lib64

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
 -lboost_program_options

ifeq ($(OS_TYPE),Darwin)
  TMPVAR := $(CXXFLAGS)
  CXXFLAGS := $(filter-out -fopenmp, $(TMPVAR))

  TMPVAR := $(LDFLAGS)
  LDFLAGS := $(filter-out -static-libgcc, $(TMPVAR))

  TMPVAR := $(LIBS)
  TMPVAR := $(filter-out -lgomp, $(TMPVAR))
  LIBS := $(filter-out -lrt, $(TMPVAR))
endif

BUILD := ./build/self
BUILDPUB := ./build/public
BUILD3RD := ./build/thirdparty
BUILDCSV := ./build/thirdparty/csv
$(shell mkdir -p $(BUILD))
$(shell mkdir -p $(BUILDPUB))
$(shell mkdir -p $(BUILD3RD))
$(shell mkdir -p $(BUILDCSV))

DIR := . ../public

SRC := $(foreach d, $(DIR), $(wildcard $(d)/*.cc))

RELEASE_OBJ := $(patsubst %.cc, $(BUILD)/%.o, $(SRC))

DEBUG_OBJ := $(patsubst %.cc, $(BUILD)/%.d.o, $(SRC))

RELEASE_BIN := pod_publish
DEBUG_BIN := pod_publishd
ifeq ($(OS_TYPE),Windows)
  RELEASE_BIN = $(addsuffix .exe,$(RELEASE_BIN))
  DEBUG_BIN = $(addsuffix .exe,$(DEBUG_BIN))
endif

all: $(RELEASE_BIN) $(DEBUG_BIN)

$(BUILD)/%.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(RELEASE_FLAGS) $(MCL_FLAGS) $(BOOST_FLAGS) -o $@


$(BUILD)/%.d.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(DEBUG_FLAGS) $(MCL_FLAGS) $(BOOST_FLAGS) -o $@

$(DEBUG_BIN): $(DEBUG_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

$(RELEASE_BIN): $(RELEASE_OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LIBS)

clean:
	find . -name "*.o" -delete
	rm -rf $(RELEASE_BIN) $(DEBUG_BIN)
	rm -rf build

.PHONY :
