## Compiler Support
Clang 17
-DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
GCC 13

TODO: look up MSVC ver

https://en.cppreference.com/w/cpp/compiler_support/23


TODO: clean up build steps.. They are a bit outdated now since windows requires visual studio toolchain

# Build
### Pre steps
* Environment variables 'clang' and 'clang++' must hold the path to the respective executables
* /cppcheck.sh must be executable -> sudo chmod +x cppcheck.sh
* Install cppcheck -> sudo apt install cppcheck
* Install cmake -> sudo apt install cmake

## Building with Ninja
* sudo apt install ninja-build
* cd root
* mkdir build
* cd build
#### Debug Configuration
* cmake .. -G Ninja -D CMAKE_BUILD_TYPE=Debug -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
#### ReleaseWithDebugInfo Configuration
* cmake .. -G Ninja -D CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
#### MinSizeRelease Configuration
* cmake .. -G Ninja -D CMAKE_BUILD_TYPE=MinSizeRel -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
#### Release Configuration
* cmake .. -G Ninja -D CMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++
#### Compile and Link
ninja

## Removing Cppcheck as build dependency
Cppcheck can be removed as a custom build target in case it causes troubles (Visual Studio does not like it e.g.). Or in case you just don't want to install it..
* open root/src/CMakeLists.txt
* Remove line 5 - "add_dependencies(${MAIN_PROJECT} cppcheck)"

