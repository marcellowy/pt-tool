set(PROJECT_NAME "chadtest1")

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

add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE chadtest_lib)