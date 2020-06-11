cmake_minimum_required(VERSION 3.6)

project(vnv LANGUAGES CXX C)

set(PROJECT_VERSION 1.0.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Injection)

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

option(WITH_MPI "Build using MPI" ON)

if(WITH_MPI)
  find_package(MPI REQUIRED)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type (default RelWitDebInfo)")
endif()

option(BUILD_EXAMPLES "Build the examples" ON)
option(WITH_EXTRACTION "Build the extraction" ON)

find_package(Clang)
if(NOT WITH_EXTRACTION OR NOT Clang_FOUND)
  message(
    WARNING
      "Clang is required to auto generate the registration.cpp file.
       The VnV Library will be built using the stored registration file. Without Clang,
       all registration must be completed manually.")
endif()

include("${CMAKE_SOURCE_DIR}/cmake/Registration.cmake")

add_subdirectory(extraction)
add_subdirectory(injection)
add_subdirectory(generation)

option(BUILD_TESTS "Build the test runner" ON)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()

if(BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
add_subdirectory(docs)

if(Clang_FOUND)
  install(
    TARGETS injection vnv-matcher
    EXPORT injection-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)

  set_target_properties(vnv-matcher PROPERTIES EXPORT_NAME Extraction)

else()
  install(
    TARGETS injection
    EXPORT injection-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
endif()

set_target_properties(injection PROPERTIES EXPORT_NAME Injection)

install(
  EXPORT injection-targets
  FILE InjectionTargets.cmake
  NAMESPACE Injection::
  DESTINATION lib/cmake/Injection)

install(FILES ${CMAKE_CURRENT_LIST_DIR}/injection/include/VnV.h
        DESTINATION include)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/injection/include/
        DESTINATION include)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/InjectionConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

# Install the config, configversion and custom find modules
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfig.cmake
        ${CMAKE_CURRENT_LIST_DIR}/cmake/Registration.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfigVersion.cmake
  DESTINATION ${INSTALL_CONFIGDIR})

export(
  EXPORT injection-targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/InjectionTargets.cmake
  NAMESPACE Injection::)
export(PACKAGE Injection)
