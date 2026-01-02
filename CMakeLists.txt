# CMakeList.txt: test 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

set(PT_TOOL_PROJECT_NAME "chadtest")
set(PT_TOOL_LIB_NAME "chadtest_lib")

project (${PT_TOOL_PROJECT_NAME})

file(GLOB SOURCE_FILE
    ${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/site/mteam/*.h
	${CMAKE_SOURCE_DIR}/src/site/mteam/*.cpp
    ${CMAKE_SOURCE_DIR}/utils/*.h
    ${CMAKE_SOURCE_DIR}/utils/*.cpp
)

include_directories(
	${CMAKE_SOURCE_DIR}/3rdparty/include
	${CMAKE_SOURCE_DIR}/utils
    ${CMAKE_SOURCE_DIR}/src/site
)

find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)
find_package(CURL REQUIRED)
find_package(ZenLib CONFIG REQUIRED)
find_package(MediaInfoLib CONFIG REQUIRED)
find_package(FFMPEG REQUIRED)
find_package(croncpp CONFIG REQUIRED)
find_package(tomlplusplus CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(httplib CONFIG REQUIRED)

# 将源代码添加到此项目的可执行文件。
add_executable(${PT_TOOL_PROJECT_NAME} ${SOURCE_FILE})
add_library(${PT_TOOL_LIB_NAME} STATIC ${SOURCE_FILE})

set_target_properties(${PT_TOOL_LIB_NAME} PROPERTIES 
    OUTPUT_NAME ${PT_TOOL_PROJECT_NAME}
)

add_library(common_deps INTERFACE)
target_link_libraries(common_deps INTERFACE 
    OpenSSL::SSL
    OpenSSL::Crypto
    ZLIB::ZLIB
    CURL::libcurl
    zen
    mediainfo
    croncpp::croncpp
    tomlplusplus::tomlplusplus
    nlohmann_json::nlohmann_json
    spdlog::spdlog_header_only
    # spdlog::spdlog
    httplib::httplib
    ${Stb_INCLUDE_DIR}
    ${FFMPEG_LIBRARY_DIRS}
    ${FFMPEG_LIBRARIES}
)

macro(setup name)
    if(MSVC)
        target_compile_options(${name} PRIVATE /utf-8)
	    # if use ninja
	    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            set_target_properties(${name} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreadedDebug"
            )
        else()
            set_target_properties(${name} PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreaded"
            )
        endif()
    else()
        if(CMAKE_BUILD_TYPE STREQUAL "Debug")
            target_compile_definitions(${name} PRIVATE DEBUG=1)
        endif()
    endif()
        
    if(WIN32)
        add_compile_definitions(${name} PRIVATE -DUNICODE -D_UNICODE)
    endif()

        target_link_libraries(${name} PRIVATE common_deps)
endmacro()

setup(${PT_TOOL_PROJECT_NAME})
setup(${PT_TOOL_LIB_NAME})

set(FILES_TO_COPY 
    ${CMAKE_SOURCE_DIR}/src/config.toml
    ${CMAKE_SOURCE_DIR}/src/config_dev.toml
    ${CMAKE_SOURCE_DIR}/src/config_tv_name.toml
)

add_custom_command(
    TARGET ${PT_TOOL_PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${FILES_TO_COPY}
        $<TARGET_FILE_DIR:${PT_TOOL_PROJECT_NAME}>  # COPY TO TARGET DIR
)

## Add tests
enable_testing()
add_subdirectory(tests)
