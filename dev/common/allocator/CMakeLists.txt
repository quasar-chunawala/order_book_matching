cmake_minimum_required(VERSION 3.25)
project(memory_pool_allocator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable debug information
set(CMAKE_BUILD_TYPE Debug)

set(source_files
    src/main.cpp
)

add_library(memory_pool_allocator INTERFACE)

# add_executable(memory_pool_allocator ${source_files})

target_include_directories(memory_pool_allocator 
    PUBLIC include
    PUBLIC ../memory_pool/include
)