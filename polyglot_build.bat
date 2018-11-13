
echo off
echo ; set +v # > NUL
echo ; function GOTO { true; } # > NUL

GOTO WIN
# btw this is from https://gist.github.com/prail/24acc95908e581722c0e9df5795180f6

cmake -Bbuild_linux/libressl -H"thirdparty/libressl-2.8.2" -DCMAKE_BUILD_TYPE=Debug -DLIBRESSL_SKIP_INSTALL=ON -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF
cmake --build build_linux/libressl/ -- -j$(nproc)
cmake -Bbuild_linux -H. -DCMAKE_BUILD_TYPE=Debug
cmake --build build_linux/ -- -j$(nproc)
exit 0

:WIN
cmake -Bbuild_win32/libressl -H"thirdparty/libressl-2.8.2" -DCMAKE_BUILD_TYPE=Debug -DLIBRESSL_SKIP_INSTALL=ON -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF -G"Visual Studio 15 2017 Win64"
cmake --build build_win32/libressl/ --config Debug
cmake -Bbuild_win32 -H. -DCMAKE_BUILD_TYPE=Debug -G"Visual Studio 15 2017 Win64"
cmake --build build_win32/ --config Debug
