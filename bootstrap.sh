#!/bin/bash

mkdir -p ~/.cnpm/cpp_modules
mkdir -p cpp_modules
if [ ! -d ~/.cnpm/cpp_modules/urlcpp ] ; then
  git clone https://github.com/rimmartin/urlcpp.git  ~/.cnpm/cpp_modules/urlcpp
  ln -s ~/.cnpm/cpp_modules/urlcpp cpp_modules/urlcpp
fi
if [ ! -d ~/.cnpm/cpp_modules/CLI11 ] ; then
  git clone https://github.com/CLIUtils/CLI11.git  ~/.cnpm/cpp_modules/CLI11
  ln -s ~/.cnpm/cpp_modules/CLI11 cpp_modules/CLI11
fi
if [ ! -d ~/.cnpm/cpp_modules/openssl ] ; then
git clone -b OpenSSL_1_1_1w https://github.com/openssl/openssl.git  ~/.cnpm/cpp_modules/openssl
ln -s ~/.cnpm/cpp_modules/openssl cpp_modules/openssl
fi
if [ ! -d ~/.cnpm/cpp_modules/libgit2 ] ; then
  git clone https://github.com/libgit2/libgit2.git  ~/.cnpm/cpp_modules/libgit2
  ln -s ~/.cnpm/cpp_modules/libgit2 cpp_modules/libgit2
fi
if [ ! -d ~/.cnpm/cpp_modules/zlib ] ; then
  git clone https://github.com/madler/zlib.git  ~/.cnpm/cpp_modules/zlib
  ln -s ~/.cnpm/cpp_modules/zlib cpp_modules/zlib
fi
if [ ! -d ~/.cnpm/cpp_modules/json-thresher ] ; then
  git clone https://github.com/DataDriven-CAM/json-thresher.git  ~/.cnpm/cpp_modules/json-thresher
  ln -s ~/.cnpm/cpp_modules/json-thresher cpp_modules/json-thresher
fi
if [ ! -d ~/.cnpm/cpp_modules/c-plus-plus-serializer ] ; then
  git clone https://github.com/goblinhack/c-plus-plus-serializer.git  ~/.cnpm/cpp_modules/c-plus-plus-serializer
  ln -s ~/.cnpm/cpp_modules/c-plus-plus-serializer cpp_modules/c-plus-plus-serializer
fi

which cmake
which g++
echo "make zlib"
#cd cpp_modules/zlib && pwd && which cmake #&& make clean #&& cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=dist . && make install
#cd ../..
echo "make openssl"
#cd cpp_modules/openssl && ./config shared --prefix=`pwd`/openssl --openssldir=`pwd`/openssl/openssl && make && make install
#cd ../..
echo "make zurlcpp"
#cd cpp_modules/urlcpp && mkdir -p test && cnpm install && pwd && make -f Makefile all
#cd ../..
echo "make json-thresher"
#cd cpp_modules/json-thresher && pwd && make -f Makefile all
#cd ../..
echo "make libgit2"
#export CXXFLAGS="  ${CXXFLAGS}"
#export LDFLAGS=" -L/usr/lib/x86_64-linux-gnu -ldl ${LDFLAGS}"
#cd cpp_modules/libgit2 && rm -rf build && mkdir -p build && cd build && ccmake -DCMAKE_INSTALL_PREFIX=`pwd`/../dist -DCMAKE_BUILD_TYPE=Release -DGIT_OPENSSL_DYNAMIC=ON -DOPENSSL_INCLUDE_DIR=`pwd`/../../openssl/openssl/include -DOPENSSL_CRYPTO_LIBRARY=`pwd`/../../openssl/openssl/lib/libcrypto.so  -DOPENSSL_SSL_LIBRARY=`pwd`/../../openssl/openssl/lib/libssl.so  .. && cmake --build . && make install
#cd ../../..

#make -j3

printf '\a'