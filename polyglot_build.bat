
echo off
echo ; set +v # > NUL
echo ; function GOTO { true; } # > NUL

GOTO WIN
# btw this is from https://gist.github.com/prail/24acc95908e581722c0e9df5795180f6

# cmake -Bbuild_linux/zlib -H"thirdparty/zlib" -DCMAKE_BUILD_TYPE=Debug -DAMD64=ON
# cmake --build build_linux/zlib/ -- -j$(nproc)
# cmake -Bbuild_linux/libressl -H"thirdparty/libressl" -DCMAKE_BUILD_TYPE=Debug -DLIBRESSL_SKIP_INSTALL=ON -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF
# cmake --build build_linux/libressl/ -- -j$(nproc)
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ -- -j$(nproc)
exit 0

:WIN
REM cmake -Bbuild_win32/zlib -H"thirdparty/zlib" -DCMAKE_BUILD_TYPE=Debug -DAMD64=ON -G"Visual Studio 15 2017 Win64"
REM cmake --build build_win32/zlib/ --config Debug
REM cmake -Bbuild_win32/libressl -H"thirdparty/libressl" -DCMAKE_BUILD_TYPE=Debug -DLIBRESSL_SKIP_INSTALL=ON -DLIBRESSL_APPS=OFF -DLIBRESSL_TESTS=OFF -G"Visual Studio 15 2017 Win64"
REM cmake --build build_win32/libressl/ --config Debug
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug -G"Visual Studio 15 2017 Win64"
cmake --build build/ --config Debug
