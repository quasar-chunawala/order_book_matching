cmake_minimum_required(VERSION 3.27)

# Project
project(order_book)

# Set the C++ language standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED 23)

# set include directories
set(INCLUDE_DIRECTORIES
    ../../include/order_book/
)

# Add source files
set(SOURCE_FILES 
    market_data_manager.cpp
    order_book.cpp
    price_level.cpp
)

# Set output directory for all binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}) # For static libraries

add_library(order_book STATIC ${SOURCE_FILES})

# Specify include directories for the target
target_include_directories(order_book PUBLIC ${INCLUDE_DIRECTORIES})

# Add AddressSanitizer and gcov flags conditionally
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Building the order_book target in Debug mode...")
    if(MSVC)
        target_compile_options(order_book PRIVATE /fsanitize=address /Zi /MD)
        target_link_options(order_book PRIVATE /fsanitize=address)
    else()
        target_compile_options(order_book PRIVATE --coverage -fsanitize=address -g)
        target_link_options(order_book PRIVATE --coverage -fsanitize=address)
    endif()
endif()
