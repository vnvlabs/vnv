
#Add the adios engine to the target
option(WITH_ADIOS_ENGINE "Use the Adios Engine" ON)
option(WITH_BUILT_IN_TESTS "Build the VnV test libraries" ON)

#Add the third party targets ( json-schema-validator TODO github-link)
add_subdirectory(third-party)

add_subdirectory(clang)

add_library(injection SHARED "")

set_target_properties(injection
                      PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION 1)

add_library(Injection::Injection ALIAS injection)

#Add the Base files to the injection target
target_sources(injection PRIVATE
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-injection.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-output.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-parser.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-runtime.cpp                        
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-utils.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-testing.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-unit-tester.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-serializer.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-schema.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-dist-utils.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-cinterface.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-options-parser.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/vv-logging.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/vv-registration.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/engines/vv-debug-engine.cpp)


target_link_libraries(injection PRIVATE dl)
target_link_libraries(injection PUBLIC jsv::jsv)
target_link_libraries(injection PRIVATE xxhash::xxhash)


if ( WITH_BUILT_IN_TESTS)
target_sources(injection PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/src/tests/provenance.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/tests/stout-reroute.cpp)
endif()


target_include_directories(injection PRIVATE
                            ${CMAKE_CURRENT_LIST_DIR}/include/base
                            ${CMAKE_CURRENT_LIST_DIR}/include/engines
                            PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                            $<INSTALL_INTERFACE:include>
                          )
                     
if(WITH_ADIOS_ENGINE)
find_package(ADIOS2)
if (ADIOS2_FOUND) 
  target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/engines/vv-adios.cpp)
  target_link_libraries(injection PRIVATE adios2::adios2 )
  target_compile_definitions(injection PRIVATE WITH_MPI=1)
endif()
endif()

include(GNUInstallDirs)
set(INSTALL_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/Injection)

install(TARGETS injection
        EXPORT injection-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
       )

set_target_properties(injection PROPERTIES 
  EXPORT_NAME Injection
)

install(EXPORT injection-targets 
  FILE
    InjectionTargets.cmake
  NAMESPACE
    Injection::
  DESTINATION lib/cmake/Injection
    )

  install(FILES 
     ${CMAKE_CURRENT_LIST_DIR}/include/VnV.h
     ${CMAKE_CURRENT_LIST_DIR}/include/VnV-Interfaces.h
     DESTINATION include)


#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/InjectionConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfig.cmake
    INSTALL_DESTINATION ${INSTALL_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/InjectionConfigVersion.cmake
    DESTINATION ${INSTALL_CONFIGDIR}
)
export(EXPORT injection-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/InjectionTargets.cmake NAMESPACE Injection::)

export(PACKAGE Injection)


