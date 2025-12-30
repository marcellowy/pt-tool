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

set(PROJECT_NAME "chadtest")

project (${PROJECT_NAME})

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

# 将源代码添加到此项目的可执行文件。
add_executable(${PROJECT_NAME} ${SOURCE_FILE})

if(MSVC)

    target_compile_options(${PROJECT_NAME} PRIVATE /utf-8)

	# if use ninja
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreadedDebug"
        )
    else()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreaded"
        )
    endif()
endif()

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::Crypto)

find_package(ZLIB REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)

find_package(ZenLib CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE zen)

find_package(MediaInfoLib CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE mediainfo)

find_package(FFMPEG REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})

if(WIN32)
    add_compile_definitions(${PROJECT_NAME} PRIVATE -DUNICODE -D_UNICODE)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DNO_UNICODE)    
endif()

set(FILES_TO_COPY 
    ${CMAKE_SOURCE_DIR}/src/config.toml
    ${CMAKE_SOURCE_DIR}/src/config_dev.toml
    ${CMAKE_SOURCE_DIR}/src/config_tv_name.toml
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${FILES_TO_COPY}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>  # 可执行文件所在目录
)