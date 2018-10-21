
echo off
echo ; set +v # > NUL
echo ; function GOTO { true; } # > NUL

mkdir build

GOTO WIN
# btw this is from https://gist.github.com/prail/24acc95908e581722c0e9df5795180f6
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug
cmake --build build/ -- -j$(nproc)
exit 0

:WIN
cmake -Bbuild -H. -DCMAKE_BUILD_TYPE=Debug -G"Visual Studio 15 2017 Win64"
cmake --build build/ --config Debug
