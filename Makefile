#std-c++11
CXX = g++
CXXFLAGS = -Wall -O2 -Wextra -Wno-unused-local-typedefs  -Werror -Wno-deprecated-declarations -std=c++11
ifeq "$(GCCVERSION)" "1"
  CXXFLAGS += -Wno-error=misleading-indentation
endif

ROOT=`root-config --cflags --glibs`
INCLUDE=-I $(PWD)
MKDIR_BIN=mkdir -p $(PWD)/bin

all: mkdirBin compareHADD

mkdirBin:
	$(MKDIR_BIN)

compareHADD: src/compareHADD.C
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(ROOT) -o bin/compareHADD.exe src/compareHADD.C

clean:
	rm -f *~
	rm -f include/*~
	rm -f include/#*#
	rm -f src/*~
	rm -f src/#*#
	rm -f bin/*.exe
	rmdir bin || true