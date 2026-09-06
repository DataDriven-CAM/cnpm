OPT_FLAGS ?= -O3

all: LDFLAGS= -fuse-ld=gold -L../cpp_modules/json-thresher -L../ -ljsonthresher -pthread
all: build/src/semantic_test.o
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $(wildcard build/src/*.o) -o test  

build/src/semantic_test.o: CXXFLAGS= -DNDEBUG $(OPT_FLAGS) -pthread -std=c++26 -I../src -I../cpp_modules/doctest -I../cpp_modules/expected/include -I../cpp_modules/graph-v3/include -I../cpp_modules/fmt/dist/include -MMD
build/src/semantic_test.o: src/semantic_test.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/semantic_test.o src/semantic_test.cpp 
