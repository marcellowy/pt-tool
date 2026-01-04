set(PROJECT_NAME "unit_tests")

project (${PROJECT_NAME})

file(GLOB SOURCE_FILE
    ${CMAKE_SOURCE_DIR}/tests/*.cpp
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
    ${CMAKE_SOURCE_DIR}/tests/test.jpeg
    ${CMAKE_SOURCE_DIR}/src/config.toml
    ${CMAKE_SOURCE_DIR}/src/config_tv_name.toml
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${FILES_TO_COPY}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>  # 可执行文件所在目录
)

set(DEV_CONFIG ${CMAKE_SOURCE_DIR}/src/config_dev.toml)
if (EXISTS ${DEV_CONFIG})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${DEV_CONFIG} $<TARGET_FILE_DIR:${PROJECT_NAME}>  # COPY TO TARGET DIR
    )
endif ()

add_test(AllTestsInMain main)

include(GoogleTest)
gtest_discover_tests(${PROJECT_NAME})

