# PT TOOL

C++ library for M-Team TPTV Team

## Dependencies
- CMake
- OpenSSL
- Zlib
- MediaInfo
- ffmpeg
- tomlplusplus
- Stb
- nlohmann
- spdlog
- httplib
- LibtorrentRasterbar
- TgBot
- Libcurl

We use vcpkg to minimize platform-specific differences in dependencies.

Thank you to the [vcpkg](https://vcpkg.io/) project.

## Library installation on Linux via vcpkg

You can install dependencies on Debian-based distributive with these commands:
```shell
sudo apt install gcc g++ cmake pkg-config autoconf ninja-build python3 nasm

git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && ./bootstrap-vcpkg.sh
```

Add VCPKG_ROOT to system environment
```shell
export VCPKG_ROOT=/path/to/your/vcpkg
export PATH=$VCPKG_ROOT:$PATH
```

Install Dependencies
```shell
vcpkg install curl ffmpeg libzen libmediainfo openssl gtest croncpp tomlplusplus stb nlohmann-json spdlog cpp-httplib libtorrent tgbot-cpp
```

## Library installation on Windows via vcpkg
```shell
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat
```

Install Dependencies
```shell
vcpkg install curl ffmpeg libzen libmediainfo openssl gtest croncpp tomlplusplus stb nlohmann-json spdlog cpp-httplib libtorrent tgbot-cpp
```
Add VCPKG_ROOT=/path/to/your/vcpkg to system environment

## CMake
Add cmake toolchain file
```shell
-DCMAKE_TOOLCHAIN_FILE=$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake
```
if you want use static library, vcpkg install add target triplet x64-windows-static (only windows) and add triplet to cmake command
```shell
# static library
vcpkg install curl:x64-windows-static ...

# cmake add build param dynamic library
-DVCPKG_TARGET_TRIPLET=x64-windows

# cmake add build param static library
-DVCPKG_TARGET_TRIPLET=x64-windows-static
 
```

## Build
### Build on Linux

### Build on Windows (Visual Studio)

### Build on Windows (mingw64)
