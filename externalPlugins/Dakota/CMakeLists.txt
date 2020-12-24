cmake_minimum_required(VERSION 3.6)

project(DakotaVnV LANGUAGES CXX C)

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

FIND_PACKAGE(Boost REQUIRED COMPONENTS system filesystem date_time serialization program_options regex)
find_package(Injection REQUIRED)
find_package(Dakota REQUIRED)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_library(DakotaVnV SHARED "")
add_subdirectory(src)
add_subdirectory(examples)

link_vnv_file(DakotaVnV Dakota cpp)

install(
  TARGETS DakotaVnV
  EXPORT DakotaVnV-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin)

set_target_properties(DakotaVnV PROPERTIES EXPORT_NAME DakotaVnV)

install(
  EXPORT DakotaVnV-targets
  FILE DakotaVnVTargets.cmake
  NAMESPACE DakotaVnv::
  DESTINATION lib/cmake/)

# Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/DakotaVnVConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/DakotaVnVConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/DakotaVnVConfig.cmake
  INSTALL_DESTINATION ${INSTALL_CONFIGDIR})

# Install the config, configversion and custom find modules
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/DakotaVnVConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/DakotaVnVConfigVersion.cmake
        DESTINATION ${INSTALL_CONFIGDIR})

export(
  EXPORT DakotaVnV-targets
  FILE ${CMAKE_CURRENT_BINARY_DIR}/DakotaVnVTargets.cmake
  NAMESPACE DakotaVnV::)
export(PACKAGE DakotaVnV)

install(TARGETS DakotaVnV DESTINATION lib/)
