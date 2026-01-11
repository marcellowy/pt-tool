#!/bin/sh
set -e
cd ..
SOURCE_DIR=`pwd`

BUILD_DIR="out/build/linux-debug"
BUILD_TYPE="Debug"

if [ "$1" = "Release" ]; then
    BUILD_DIR="out/build/linux-release"
    BUILD_TYPE="Release"
fi

echo "Build Type: $BUILD_TYPE"
echo "Build: $BUILD_DIR"

echo "Press any key to continue..."
read value

rm -rf $BUILD_DIR && mkdir -p $BUILD_DIR
cd $BUILD_DIR

# configure
cmake -G Ninja \
-DCMAKE_C_COMPILER=/usr/bin/gcc \
-DCMAKE_CXX_COMPILER=/usr/bin/g++ \
-DCMAKE_BUILD_TYPE=$BUILD_TYPE \
-DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" \
-S $SOURCE_DIR \
-B .

# build
ninja -v