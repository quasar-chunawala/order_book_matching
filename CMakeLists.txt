cmake_minimum_required(VERSION 3.27)

# Project name
project(order_book_matching)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable testing
enable_testing()

# Add GoogleTest to the project
add_subdirectory(ext/googletest)

# Global include directories for all test projects
include_directories(
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include
)

add_subdirectory(test/spsc_queue_tests)
add_subdirectory(test/memory_pool_tests)