
cmake_minimum_required(VERSION 3.6)

project(vnv LANGUAGES CXX C)

set(PROJECT_VERSION 1.0.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 14)


if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type (default RelWitDebInfo)")
endif()

option(BUILD_EXAMPLES "Build the examples" ON)
option(BUILD_TESTS "Build the test runner" ON)

add_subdirectory(injection)

if (BUILD_EXAMPLES) 
  add_subdirectory(examples)
endif()

if (BUILD_TESTS) 
  add_subdirectory(test)
endif()

add_subdirectory(docs)
