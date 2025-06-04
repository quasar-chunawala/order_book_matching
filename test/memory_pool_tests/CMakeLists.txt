cmake_minimum_required(VERSION 3.27)

# Project
project(memory_pool_tests)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

# set include directories
set(INCLUDE_DIRECTORIES
    ${gtest_SOURCE_DIR}/include
    ../../include/memory_pool/
)

# Add source files
set(SOURCE_FILES 
    memory_pool_tests.cpp
)

# Set output directory for all binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # For static libraries


add_executable(memory_pool_tests ${SOURCE_FILES})

# Link Google Test libraries to the target
target_link_libraries(memory_pool_tests gtest gtest_main)

# Specify include directories for the target
target_include_directories(memory_pool_tests PUBLIC ${INCLUDE_DIRECTORIES})

# Add AddressSanitizer and gcov flags conditionally
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building the memory_pool_tests target in Debug mode...")
    if(MSVC)
        target_compile_options(memory_pool_tests PRIVATE /fsanitize=address /Zi /MD)
        target_link_options(memory_pool_tests PRIVATE /fsanitize=address)
    else()
        target_compile_options(memory_pool_tests PRIVATE --coverage -fsanitize=address -g)
        target_link_options(memory_pool_tests PRIVATE --coverage -fsanitize=address)
    endif()
endif()

# Discover and register Google Test cases
include(GoogleTest)
gtest_discover_tests(memory_pool_tests)
