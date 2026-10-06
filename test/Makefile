OPT_FLAGS ?= -O3

all: LDFLAGS= -fuse-ld=gold -pthread
all: LDLIBS=../cpp_modules/urlcpp/liburlcpp.a  ../cpp_modules/json-thresher/libjsonthresher.a 
all: build/src/semantic_test.o
	@mkdir -p $(@D)
	$(CXX) -o test  $(wildcard build/src/*.o) $(LDFLAGS) $(LDLIBS)  

build/src/semantic_test.o: CXXFLAGS= -DNDEBUG $(OPT_FLAGS) -pthread -std=c++26 -I../src -I../cpp_modules/doctest -I../cpp_modules/expected/include -I../cpp_modules/graph-v3/include -I../cpp_modules/fmt/dist/include -MMD
build/src/semantic_test.o: src/semantic_test.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/semantic_test.o src/semantic_test.cpp 

clean:
	rm -rf build
