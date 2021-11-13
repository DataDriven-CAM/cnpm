
main: build/src/npm/Installation.o build/src/npm/Addition.o build/src/npm/Removal.o build/src/npm/WebGetter.o build/src/main.o

build/src/main.o: build/src/npm/Installation.o build/src/npm/Addition.o build/src/npm/Removal.o build/src/npm/WebGetter.o

CXXFLAGS=-std=c++2b -DNDEBUG -O3 -I./src -I./cpp_modules/json/include -I./cpp_modules/libgit2/dist/include -MMD
build/src/npm/Installation.o: src/npm/Installation.cpp
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Installation.o src/npm/Installation.cpp

CXXFLAGS=-std=c++2b -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/json/include -I./cpp_modules/libgit2/dist/include -MMD
build/src/npm/Addition.o: src/npm/Addition.cpp
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Addition.o src/npm/Addition.cpp

CXXFLAGS=-std=c++2b -DNDEBUG -O3 -fconcepts -I./src -I./cpp_modules/json/include -I./cpp_modules/libgit2/dist/include -MMD
build/src/npm/Removal.o: src/npm/Removal.cpp
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/Removal.o src/npm/Removal.cpp

CXXFLAGS=-std=c++2b -DNDEBUG -O3 -I./src -I./cpp_modules/c-plus-plus-serializer -I./cpp_modules/openssl/openssl/include -MMD
build/src/npm/WebGetter.o: src/npm/WebGetter.cpp
	$(CXX) $(CXXFLAGS) -c -o build/src/npm/WebGetter.o src/npm/WebGetter.cpp

LDFLAGS= -Wl,-rpath,"$$ORIGIN/cpp_modules/openssl/openssl/lib64":"`pwd`/cpp_modules/openssl/openssl/lib64":"`pwd`/cpp_modules/libgit2/dist/lib" -L./cpp_modules/libgit2/dist/lib -L./cpp_modules/urlcpp -L./cpp_modules/openssl/openssl/lib64 -Lcpp_modules/zlib/dist/lib -lurlcpp -lgit2 -lcrypto -lssl -lz
CXXFLAGS=-std=c++2b -O3 -fpermissive -I./src -I./cpp_modules/c-plus-plus-serializer -I./cpp_modules/urlcpp  -I./cpp_modules/openssl/openssl/include -I./cpp_modules/CLI11/include  -I./cpp_modules/json/include  -I./cpp_modules/libgit2/dist/include
build/src/main.o: src/main.cpp
	$(CXX) $(CXXFLAGS) -o build/src/main.o -c src/main.cpp
	$(CXX) $(LDFLAGS) -o cnpm $(wildcard build/src/*.o) $(wildcard build/src/npm/*.o)
