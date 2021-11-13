#!/bin/bash

mkdir -p ~/.cnpm/cpp_modules
git clone https://github.com/rimmartin/urlcpp.git  ~/.cnpm/cpp_modules/urlcpp
git clone https://github.com/CLIUtils/CLI11.git  ~/.cnpm/cpp_modules/CLI11
git clone https://github.com/openssl/openssl\#OpenSSL_1_1_1l.git  ~/.cnpm/cpp_modules/openssl
git clone https://github.com/libgit2/libgit2.git  ~/.cnpm/cpp_modules/libgit2
git clone https://github.com/madler/zlib.git  ~/.cnpm/cpp_modules/zlib
git clone https://github.com/goblinhack/c-plus-plus-serializer.git  ~/.cnpm/cpp_modules/c-plus-plus-serializer

cd cpp_modules/zlib && make clean && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=dist . && make install
cd ../..
cd cpp_modules/openssl && ./config shared --prefix=`pwd`/openssl --openssldir=`pwd`/openssl/openssl && make && make install
cd ../..
cd cpp_modules/urlcpp && mkdir -p test && cnpm install && pwd && make -f Makefile all
cd ../..
mkdir -p build && cd build && ccmake -DCMAKE_INSTALL_PREFIX=`pwd`/../dist -DCMAKE_BUILD_TYPE=Release -DOPENSSL_CRYPTO_LIBRARY=`pwd`/../../openssl/openssl/lib64/libcrypto.so -DOPENSSL_INCLUDE_DIR=`pwd`/../../openssl/openssl/include  -DOPENSSL_SSL_LIBRARY=`pwd`/../../openssl/openssl/lib64/libssl.so  .. && cmake --build . && make install
cd ../../..

make -j3
