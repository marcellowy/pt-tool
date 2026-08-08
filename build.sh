#!/bin/sh

mkdir -p build && cd build

# 关键在最后那个 -DCMAKE_TOOLCHAIN_FILE 参数，它会让 CMake 自动去读你项目地下的 vcpkg_installed
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/usr/local/vcpkg/scripts/buildsystems/vcpkg.cmake

cmake --build . --config Release
