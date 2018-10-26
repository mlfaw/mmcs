
echo off
echo ; set +v # > NUL
echo ; function GOTO { true; } # > NUL

mkdir build
mkdir build/libressl

GOTO WIN
# btw this is from https://gist.github.com/prail/24acc95908e581722c0e9df5795180f6

cmake -Bbuild/libressl -H"thirdparty/libressl-2.8.2" -DCMAKE_BUILD_TYPE=Debug -DLIBRESSL_SKIP_INSTALL=ON -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF
cmake --build build/libressl/ -- -j$(nproc)
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ -- -j$(nproc)
exit 0

:WIN
cmake -Bbuild/libressl -H"thirdparty/libressl-2.8.2" -DCMAKE_BUILD_TYPE=Debug -DLIBRESSL_SKIP_INSTALL=ON -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF -G"Visual Studio 15 2017 Win64"
cmake --build build/libressl/ --config Debug
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug -G"Visual Studio 15 2017 Win64"
cmake --build build/ --config Debug
