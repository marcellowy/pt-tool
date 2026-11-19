#!/bin/sh

#rm -rf build
mkdir -p build && cd build

# build
cmake .. -DENABLE_GOOGLE_TEST=ON -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake
#cmake --build . --config Release --target unit_tests
cmake --build . --config Release