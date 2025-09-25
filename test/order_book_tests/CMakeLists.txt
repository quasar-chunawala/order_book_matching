cmake_minimum_required(VERSION 3.27)

# Project
project(order_book_tests)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

# set include directories
set(INCLUDE_DIRECTORIES
    ${gtest_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/order_book
)

# Add source files
set(SOURCE_FILES 
    order_book_tests.cpp
)

# Set output directory for all binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # For static libraries


add_executable(order_book_tests ${SOURCE_FILES})

# Link Google Test libraries to the target
target_link_libraries(order_book_tests gtest gtest_main order_book)

# Specify include directories for the target
target_include_directories(order_book_tests PUBLIC ${INCLUDE_DIRECTORIES})

# Add AddressSanitizer and gcov flags conditionally
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building the order_book_tests target in Debug mode...")
    if(MSVC)
        target_compile_options(order_book_tests PRIVATE /fsanitize=address /Zi /MD)
        target_link_options(order_book_tests PRIVATE /fsanitize=address)
    else()
        target_compile_options(order_book_tests PRIVATE --coverage -fsanitize=address -g)
        target_link_options(order_book_tests PRIVATE --coverage -fsanitize=address)
    endif()
endif()

# Discover and register Google Test cases
include(GoogleTest)
gtest_discover_tests(order_book_tests)
