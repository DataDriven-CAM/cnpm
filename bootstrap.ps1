
mkdir ~/.cnpm/cpp_modules -ea 0
mkdir cpp_modules -ea 0
git clone https://github.com/rimmartin/urlcpp.git  $Env:UserProfile/.cnpm/cpp_modules/urlcpp
New-Item -Path cpp_modules/urlcpp -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/urlcpp
git clone https://github.com/CLIUtils/CLI11.git  $Env:UserProfile/.cnpm/cpp_modules/CLI11
New-Item -Path cpp_modules/CLI11 -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/CLI11
git clone https://github.com/madler/zlib.git  $Env:UserProfile/.cnpm/cpp_modules/zlib
New-Item -Path cpp_modules/zlib -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/zlib
git clone https://github.com/goblinhack/c-plus-plus-serializer.git  $Env:UserProfile/.cnpm/cpp_modules/c-plus-plus-serializer
New-Item -Path cpp_modules/c-plus-plus-serializer -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/c-plus-plus-serializer
git clone https://github.com/DataDriven-CAM/json-thresher.git  $Env:UserProfile/.cnpm/cpp_modules/json-thresher
New-Item -Path cpp_modules/json-thresher -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/json-thresher
git clone https://github.com/goblinhack/c-plus-plus-serializer.git  $Env:UserProfile/.cnpm/cpp_modules/c-plus-plus-serializer
New-Item -Path cpp_modules/c-plus-plus-serializer -ItemType SymbolicLink -Target $Env:UserProfile/.cnpm/cpp_modules/c-plus-plus-serializer

#cmake -G
mkdir -ea 0 cpp_modules/json-thresher/build
cd cpp_modules/json-thresher/ && pwd && mingw32-make.exe -j 3 -f Makefile all  && pwd && cd ../..
mkdir -ea 0 cpp_modules/zlib/build
cd cpp_modules/zlib/build && cmake -G "Unix Makefiles" "-DCMAKE_BUILD_TYPE=Release" "-DCMAKE_INSTALL_PREFIX=$(pwd)/dist" .. && make install && cd ../../..
cd cpp_modules/urlcpp && pwd && make -f Makefile all && cd ../..

g++ --version
make -j3 

pwd
./cnpm.exe --help
