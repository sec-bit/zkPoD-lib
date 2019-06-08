#sudo apt-get install libomp-dev

CXX11_HOME = /usr

THIRD_PARTY_HOME = ../thirdparty
MCL_HOME = ../thirdparty/mcl/include
YAS_HOME = ../thirdparty/yas/include
PUB_HOME = ../public

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
 -I $(CXX11_HOME)/include \
 -I $(MCL_HOME) \
 -I $(YAS_HOME) \
 -I $(THIRD_PARTY_HOME) \
 -I $(PUB_HOME) \
 -I. \

MCL_FLAGS := \
 -DMCL_DONT_USE_OPENSSL

RELEASE_FLAGS := \
 -O3 \
 -DDTREE_ICASE \
 -DNDEBUG \
 -DLINUX \
 -DMULTICORE \

DEBUG_FLAGS := \
 -O0 \
 -D_DEBUG \
 -DLINUX \
 
LDFLAGS += \
 -static-libstdc++ -static-libgcc \
 -fmerge-all-constants \
 -L${CXX11_HOME}/lib64

LIBS := \
 -lpthread \
 -lrt \
 -lgmpxx \
 -lgmp \
 -lcryptopp 

DIR := . 

SRC := $(foreach d, $(DIR), $(wildcard $(d)/*.cc))



RELEASE_OBJ := $(patsubst %.cc, %.o, $(SRC))

DEBUG_OBJ := $(patsubst %.cc, %.d.o, $(SRC))



CXX := export LD_LIBRARY_PATH=${CXX11_HOME}/lib; ${CXX11_HOME}/bin/g++

#CC := export LD_LIBRARY_PATH=${CXX11_HOME}/lib; ${CXX11_HOME}/bin/gcc



all: pod_dummyd pod_dummy

%.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(RELEASE_FLAGS) $(MCL_FLAGS) -o $@


%.d.o : %.cc
	$(CXX) -c $^ $(CXXFLAGS) $(DEBUG_FLAGS) $(MCL_FLAGS) -o $@



pod_dummyd : $(DEBUG_OBJ)
	$(CXX) $^ -o $@.exe $(LDFLAGS) $(LIBS)

pod_dummy : $(RELEASE_OBJ)
	$(CXX) $^ -o $@.exe $(LDFLAGS) $(LIBS)

clean:
	find . -regex "\(.*\.o\|.*\.exe\)" | xargs rm

.PHONY :
