cmake_minimum_required(VERSION 3.6)

project(papiTests LANGUAGES CXX C)

set(PROJECT_VERSION 1.0.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake)

if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

# Note: This line was added because I forgot to link PAPI. It makes it a
# compiler error to have undefined symbols in the shared lib.
string(APPEND CMAKE_SHARED_LINKER_FLAGS " -Wl,--no-undefined")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE
      RelWithDebInfo
      CACHE STRING "Build type (default RelWitDebInfo)")
endif()

find_package(Injection REQUIRED)
find_package(PAPI REQUIRED)

add_library(papiTest SHARED "")
add_subdirectory(src)

link_vnv_file(papiTest Papi cpp)

install(
  TARGETS papiTest
  EXPORT papiTest-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)

set_target_properties(papiTest PROPERTIES EXPORT_NAME PapiTest)

install(
  EXPORT papiTest-targets
  FILE PapiTestTargets.cmake
  NAMESPACE PapiTest::
  DESTINATION lib/cmake/)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/PapiTestConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/PapiTestConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/PapiTestConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

# Install the config, configversion and custom find modules
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/PapiTestConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/PapiTestConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR})

export(
  EXPORT papiTest-targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/PapiTestTargets.cmake
  NAMESPACE PapiTest::)
export(PACKAGE PapiTest)

install(TARGETS papiTest DESTINATION lib/)
