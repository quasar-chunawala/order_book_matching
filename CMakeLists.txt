cmake_minimum_required(VERSION 3.25)
project(tachyon VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_BUILD_TYPE Debug)

# Add Google Test
add_subdirectory(ext/googletest)

# Add subprojects
add_subdirectory(dev/common/memory_pool)
add_subdirectory(dev/common/memory_pool_tests)