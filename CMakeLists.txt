cmake_minimum_required(VERSION 3.28)
project(CGo)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fexceptions")

# Include GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()

# Main program
add_executable(CGo src/main.cpp)
target_include_directories(CGo PRIVATE include)

# GoogleTest-based tests
file(GLOB TEST_SOURCES test/*.cpp)
add_executable(GTEST ${TEST_SOURCES})
target_include_directories(GTEST PRIVATE include)
target_link_libraries(GTEST gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(GTEST)
