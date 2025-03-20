cmake_minimum_required(VERSION 3.25)
project(memory_pool_tests VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

set(source_files
    src/memory_pool_tests.cpp
)

add_executable(memory_pool_tests ${source_files})

# Include the memory_pool headers
target_include_directories(memory_pool_tests  
PUBLIC ../memory_pool/include/
PUBLIC ../../ext/googletest/include/
)

target_link_libraries(memory_pool_tests 
INTERFACE memory_pool 
PUBLIC gtest
PUBLIC gtest_main
)

enable_testing()
include(GoogleTest)
gtest_discover_tests(memory_pool_tests)