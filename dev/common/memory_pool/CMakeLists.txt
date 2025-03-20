cmake_minimum_required(VERSION 3.25.1)
project(memory_pool     # name of the project
    VERSION 1.0.0       # project version major.minor.patch
)

# Specify the C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable debug information
set(CMAKE_BUILD_TYPE Debug)

# Specify the source files
set(source_files
    src/main.cpp
    # Add other source files here
)

# add_executable(memory_pool ${source_files})
add_library(memory_pool INTERFACE)

target_include_directories(memory_pool INTERFACE include)