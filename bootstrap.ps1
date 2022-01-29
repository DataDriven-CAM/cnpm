
mkdir ~/.cnpm/cpp_modules -ea 0
mkdir cpp_modules -ea 0
git clone https://github.com/rimmartin/urlcpp.git  $Env:UserProfile/.cnpm/cpp_modules/urlcpp
New-Item -Path cpp_modules/urlcpp -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/urlcpp
git clone https://github.com/CLIUtils/CLI11.git  $Env:UserProfile/.cnpm/cpp_modules/CLI11
New-Item -Path cpp_modules/CLI11 -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/CLI11
git clone -b OpenSSL_1_1_1l https://github.com/openssl/openssl.git  $Env:UserProfile/.cnpm/cpp_modules/openssl
New-Item -Path cpp_modules/openssl -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/openssl
git clone https://github.com/libgit2/libgit2.git  $Env:UserProfile/.cnpm/cpp_modules/libgit2
New-Item -Path cpp_modules/libgit2 -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/libgit2
git clone https://github.com/madler/zlib.git  $Env:UserProfile/.cnpm/cpp_modules/zlib
New-Item -Path cpp_modules/zlib -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/zlib
git clone https://github.com/goblinhack/c-plus-plus-serializer.git  $Env:UserProfile/.cnpm/cpp_modules/c-plus-plus-serializer
New-Item -Path cpp_modules/c-plus-plus-serializer -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/c-plus-plus-serializer

cmake -G
mkdir -ea 0 cpp_modules/zlib/build
cd cpp_modules/zlib/build && cmake -G "Unix Makefiles" "-DCMAKE_BUILD_TYPE=Release" "-DCMAKE_INSTALL_PREFIX=$(pwd)/dist" .. && make install && cd ../../..
cd cpp_modules/openssl && sh ./config -v shared "--prefix=$(pwd)/openssl" "--openssldir=$(pwd)/openssl/openssl" && make && make install && cd ../..
cd cpp_modules/urlcpp && pwd && make -f Makefile all && cd ../..
mkdir -ea 0 cpp_modules/libgit2/build
cd cpp_modules/libgit2/build && $installdir && cmake -G "Unix Makefiles" "-DCMAKE_INSTALL_PREFIX=$(pwd)/../dist" "-DCMAKE_C_COMPILER=gcc" "-DCMAKE_BUILD_TYPE=Release" "-DOPENSSL_CRYPTO_LIBRARY=$(pwd)/../../openssl/libcrypto-1_1-x64.dll" "-DOPENSSL_INCLUDE_DIR=$(pwd)/../../openssl/include"  "-DOPENSSL_SSL_LIBRARY=$(pwd)/../../openssl/libssl-1_1-x64.dll"  .. && cmake --build . && make install && cd ../../..

mkdir -ea 0 build/src/npm
g++ --version
make -j3 

pwd
./cnpm.exe --help
