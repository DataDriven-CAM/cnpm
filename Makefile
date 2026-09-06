ext=so
libprefix=
wincxx=
ifeq ($(OS),Windows_NT)
  ext=dll
  libprefix=lib
  wincxx=-DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -Ic:\msys64\mingw64\x86_64-w64-mingw32\include 
endif

LDFLAGS=-Wl,-rpath,"$$ORIGIN/cpp_modules/urlcpp" -Wl,-rpath,"/home/roger/Software/gcc-dev/dist/lib64" -L./cpp_modules/stdexec -Lcpp_modules/zlib/dist/lib -l$(libprefix)z
LDLIBS=./cpp_modules/urlcpp/liburlcpp.a  ./cpp_modules/json-thresher/libjsonthresher.a 
ifeq ($(OS),Windows_NT)
LDFLAGS+= C:/Windows/System32/ws2_32.dll
endif
all: build/src/npm/Initialization.o build/src/npm/Installation.o build/src/npm/Addition.o build/src/npm/Remodeler.o build/src/npm/Removal.o build/src/npm/Outdated.o build/src/npm/Snap.o build/src/main.o
	$(CXX) -o cnpm $(wildcard build/src/*.o) $(wildcard build/src/npm/*.o) $(LDFLAGS) $(LDLIBS)

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -I./src -I./cpp_modules/fmt/dist/include -I./cpp_modules/expected/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include -MMD
build/src/npm/Initialization.o: src/npm/Initialization.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Initialization.o src/npm/Initialization.cpp

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -I./src -I./cpp_modules/fmt/dist/include -I./cpp_modules/expected/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include  -I./cpp_modules/stdexec/include -I./cpp_modules/zlib/dist/include -MMD
build/src/npm/Installation.o: src/npm/Installation.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Installation.o src/npm/Installation.cpp

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/fmt/dist/include -I./cpp_modules/expected/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include -MMD
build/src/npm/Addition.o: src/npm/Addition.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Addition.o src/npm/Addition.cpp

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/fmt/dist/include -I./cpp_modules/expected/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include -MMD
build/src/npm/Remodeler.o: src/npm/Remodeler.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Remodeler.o src/npm/Remodeler.cpp

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/fmt/dist/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include -MMD
build/src/npm/Removal.o: src/npm/Removal.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Removal.o src/npm/Removal.cpp

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/fmt/dist/include -I./cpp_modules/expected/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include -MMD
build/src/npm/Outdated.o: build/src/npm/Outdated.o src/npm/Outdated.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Outdated.o src/npm/Outdated.cpp

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -I./src -I./cpp_modules/fmt/dist/include -I./cpp_modules/expected/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include -MMD
build/src/npm/Snap.o: src/npm/Snap.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Snap.o src/npm/Snap.cpp

CXXFLAGS=-std=c++26 -DNDEBUG -O3 -fpermissive -I./src -I./cpp_modules/c-plus-plus-serializer -I./cpp_modules/urlcpp  -I./cpp_modules/openssl/openssl/include -I./cpp_modules/CLI11/include -I./cpp_modules/fmt/dist/include -I./cpp_modules/expected/include -I./cpp_modules/graph-v3/include -I./cpp_modules/json-thresher/include  -I./cpp_modules/stdexec/include -MMD
build/src/main.o: src/main.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o build/src/main.o -c src/main.cpp


clean:
	rm -rf build
