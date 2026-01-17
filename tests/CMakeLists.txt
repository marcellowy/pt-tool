set(PROJECT_NAME "unit_tests")

project (${PROJECT_NAME})

file(GLOB SOURCE_FILE
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
        ${CMAKE_SOURCE_DIR}/tests/douban/*.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}
)

find_package(GTest CONFIG REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCE_FILE})

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

endmacro()

setup(${PROJECT_NAME})
 
target_link_libraries(${PROJECT_NAME} PRIVATE 
    GTest::gtest 
    GTest::gtest_main 
    GTest::gmock 
    GTest::gmock_main
    ${PT_TOOL_LIB_NAME}
)

set(FILES_TO_COPY 
    ${CMAKE_SOURCE_DIR}/src/config_tv_name.toml
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${FILES_TO_COPY} $<TARGET_FILE_DIR:${PROJECT_NAME}>
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # debug config file
    if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
        set(CONFIG_FILE ${CMAKE_SOURCE_DIR}/src/config_dev_linux.toml)
    elseif(CMAKE_HOST_SYSTEM_NAME MATCHES "Windows")
        set(CONFIG_FILE ${CMAKE_SOURCE_DIR}/src/config_dev_windows.toml)
    endif()

    if (EXISTS ${CONFIG_FILE})
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CONFIG_FILE} $<TARGET_FILE_DIR:${PROJECT_NAME}>/config.toml
        )
    endif ()
else()
    # release config file
    set(CONFIG_FILE ${CMAKE_SOURCE_DIR}/src/config.toml)
    if(EXISTS ${CONFIG_FILE})
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${CONFIG_FILE} $<TARGET_FILE_DIR:${PROJECT_NAME}>/config.toml
        )
    endif()
endif()

add_test(AllTestsInMain main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

