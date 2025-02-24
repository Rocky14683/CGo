cmake_minimum_required(VERSION 3.28)
project(CGo)

set(CMAKE_CXX_COMPILER "/opt/homebrew/bin/gcc")
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++23 -fcoroutines -Wno-template-id-cdtor")

# Include GoogleTest
include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)



# Main program
add_executable(CGo src/main.cpp)
target_include_directories(CGo PRIVATE include)
# link against the standard C++ library so that coroutines work
target_link_libraries(CGo stdc++)

# GoogleTest-based tests
enable_testing()

file(GLOB TEST_SOURCES test/*.cpp)
add_executable(GTEST ${TEST_SOURCES})
target_include_directories(GTEST PRIVATE include)
# link against the standard C++ library so that coroutines work
target_link_libraries(GTEST stdc++ gtest gtest_main)

include(GoogleTest)
gtest_discover_tests(GTEST)