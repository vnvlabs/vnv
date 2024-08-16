cmake_minimum_required(VERSION 3.6)
enable_language(Fortran)
project(vnv LANGUAGES CXX C Fortran)

set(PROJECT_VERSION 1.0.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/)

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

option(WITH_MPI "Build using MPI" ON)

if(WITH_MPI)
  find_package(MPI REQUIRED)
endif()

# Python is required to build the reports.
find_package(
  Python3
  COMPONENTS Interpreter Development
  REQUIRED)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type (default RelWitDebInfo)")
endif()


option(ALLERRORS OFF)
if(ALLERRORS)
  add_compile_options(-Wall -Wextra -pedantic -Wno-unused-parameter)
endif()

add_subdirectory(shared)
add_subdirectory(extraction)
add_subdirectory(third-party)

set(VNV_CURR_INSTALL ${CMAKE_BINARY_DIR})
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Registration.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Registration.cmake @ONLY)
set(VNV_CURR_INSTALL ${CMAKE_INSTALL_PREFIX})
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/Registration.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/Registration.cmake.install @ONLY)


include("${CMAKE_CURRENT_BINARY_DIR}/Registration.cmake")

install(FILES ${CMAKE_SOURCE_DIR}/scripts/register DESTINATION ${CMAKE_INSTALL_PREFIX}/bin PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE)


if (WITH_MPI)
   set(VNV_WITH_MPI "//")
else()
   set(VNV_WITH_MPI "")
endif()
configure_file(${CMAKE_CURRENT_LIST_DIR}/cmake/VnV.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/build_includes/VnV.h @ONLY)


configure_file(${CMAKE_CURRENT_LIST_DIR}/scripts/register ${CMAKE_BINARY_DIR}/bin/register)

add_subdirectory(injection)
target_include_directories(injection PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/build_includes/>)

option(BUILD_TESTS "Build the test runner" ON)
if(BUILD_TESTS)
  add_subdirectory(test)
endif()


add_subdirectory(docs)

install(
  TARGETS injection finjection
  EXPORT injection-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)

set_target_properties(injection PROPERTIES EXPORT_NAME Injection)
set_target_properties(finjection PROPERTIES EXPORT_NAME FInjection)

install(
  EXPORT injection-targets
  FILE InjectionTargets.cmake
  NAMESPACE Injection::
  DESTINATION lib/cmake)

install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/injection/include/ DESTINATION include)
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/shared/include/ DESTINATION include)
install(FILES ${CMAKE_BINARY_DIR}/build_includes/VnV.h DESTINATION include)

#Register a vnv file on install
install_vnv_registration_dir(vnv gui vnv-config.json)        


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
        ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfigVersion.cmake
  DESTINATION ${INSTALL_CONFIGDIR})
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/Registration.cmake.install
  DESTINATION ${INSTALL_CONFIGDIR}
  RENAME Registration.cmake
  )


export(
  EXPORT injection-targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/InjectionTargets.cmake
  NAMESPACE Injection::)
export(PACKAGE Injection)
