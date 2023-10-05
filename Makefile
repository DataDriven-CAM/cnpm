ext=so
libprefix=
ifeq ($(OS),Windows_NT)
  ext=dll
  libprefix=lib
endif

main: build/src/npm/Initialization.o build/src/npm/Installation.o build/src/npm/Addition.o build/src/npm/Removal.o build/src/npm/Outdated.o build/src/npm/WebGetter.o build/src/main.o

build/src/main.o: build/src/npm/Initialization.o build/src/npm/Installation.o build/src/npm/Addition.o build/src/npm/Removal.o build/src/npm/Outdated.o build/src/npm/WebGetter.o

CXXFLAGS=-std=c++23 -DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -DNDEBUG -O3 -I./src -I./cpp_modules/json-thresher/include -I./cpp_modules/libgit2/dist/include -Ic:\msys64\mingw64\x86_64-w64-mingw32\include -MMD -Wl,--allow-multiple-definition
build/src/npm/Initialization.o: src/npm/Initialization.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Initialization.o src/npm/Initialization.cpp

CXXFLAGS=-std=c++23 -DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -DNDEBUG -O3 -I./src -I./cpp_modules/json-thresher/include -I./cpp_modules/libgit2/dist/include -Ic:\msys64\mingw64\x86_64-w64-mingw32\include -MMD -Wl,--allow-multiple-definition
build/src/npm/Installation.o: src/npm/Installation.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Installation.o src/npm/Installation.cpp

CXXFLAGS=-std=c++23 -DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/json-thresher/include -I./cpp_modules/libgit2/dist/include -MMD -Wl,--allow-multiple-definition
build/src/npm/Addition.o: src/npm/Addition.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Addition.o src/npm/Addition.cpp

CXXFLAGS=-std=c++23 -DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/json-thresher/include -I./cpp_modules/libgit2/dist/include -MMD -Wl,--allow-multiple-definition
build/src/npm/Removal.o: src/npm/Removal.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Removal.o src/npm/Removal.cpp

CXXFLAGS=-std=c++23 -DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/json-thresher/include -I./cpp_modules/libgit2/dist/include -MMD -Wl,--allow-multiple-definition
build/src/npm/Outdated.o: build/src/npm/WebGetter.o src/npm/Outdated.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Outdated.o src/npm/Outdated.cpp

CXXFLAGS=-std=c++23 -DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -DNDEBUG -O3 -I./src -I./cpp_modules/c-plus-plus-serializer -I./cpp_modules/openssl/openssl/include -MMD -Wl,--allow-multiple-definition
build/src/npm/WebGetter.o: src/npm/WebGetter.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/WebGetter.o src/npm/WebGetter.cpp

build/src/main.o: build/src/npm/Outdated.o
	
LDFLAGS= -Wl,-rpath,"$$ORIGIN/cpp_modules/openssl/openssl/lib64":"`pwd`/cpp_modules/openssl/openssl/lib64":"`pwd`/cpp_modules/libgit2/dist/lib":"`pwd`/cpp_modules/urlcpp":"`pwd`/cpp_modules/json-thresher":"/home/roger/Software/gcc-13.2.0/dist/lib64" -L./cpp_modules/json-thresher -L./cpp_modules/libgit2/dist/lib -L./cpp_modules/urlcpp -L./cpp_modules/openssl/openssl/lib64 -Lcpp_modules/zlib/dist/lib  -l$(libprefix)jsonthresher -l$(libprefix)urlcpp -l$(libprefix)git2 -l$(libprefix)crypto -l$(libprefix)ssl -l$(libprefix)z -Wl,--allow-multiple-definition
ifeq ($(OS),Windows_NT)
LDFLAGS+= C:/Windows/System32/ws2_32.dll
endif
CXXFLAGS=-std=c++23 -DSYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE=2 -DNDEBUG -O3 -fpermissive -I./src -I./cpp_modules/c-plus-plus-serializer -I./cpp_modules/urlcpp  -I./cpp_modules/openssl/openssl/include -I./cpp_modules/CLI11/include  -I./cpp_modules/json-thresher/include  -I./cpp_modules/libgit2/dist/include -MMD -Wl,--allow-multiple-definition
build/src/main.o: src/main.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o build/src/main.o -c src/main.cpp
	$(CXX) $(LDFLAGS) -o cnpm $(wildcard build/src/*.o) $(wildcard build/src/npm/*.o)
