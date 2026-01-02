#set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
#include(FetchContent)
#FetchContent_Declare(
#  googletest
#  URL https://github.com/google/googletest/archive/refs/heads/main.zip
#)
#set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable(googletest)

set(PROJECT_NAME "unit_tests")

project (${PROJECT_NAME})

file(GLOB SOURCE_FILE
    ${CMAKE_SOURCE_DIR}/src/*.h
	${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/site/mteam/*.h
	${CMAKE_SOURCE_DIR}/site/mteam/*.cpp
    ${CMAKE_SOURCE_DIR}/utils/*.h
    ${CMAKE_SOURCE_DIR}/utils/*.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
)

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME} test.cpp)

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
endmacro()

setup(${PROJECT_NAME})

 
target_link_libraries(${PROJECT_NAME} PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    GTest::gmock 
    GTest::gmock_main
    chadtest_lib
)

add_test(AllTestsInMain main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})
# target_link_libraries(${PROJECT_NAME} PRIVATE chadtest_lib)

