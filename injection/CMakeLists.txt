
#Add the adios engine to the target

#Add the third party targets ( json-schema-validator TODO github-link)
add_subdirectory(third-party)

add_library(injection SHARED "")

set_target_properties(injection
                      PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION 1)

add_library(Injection::Injection ALIAS injection)


target_include_directories(injection PRIVATE
                            ${CMAKE_CURRENT_LIST_DIR}/include
                            PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                            $<INSTALL_INTERFACE:include>
                          )
target_compile_definitions(injection PRIVATE -DPACKAGENAME=VnV )

add_subdirectory(src)

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

install(FILES ${CMAKE_CURRENT_LIST_DIR}/include/VnV.h  DESTINATION include)
install(DIRECTORY
         ${CMAKE_CURRENT_LIST_DIR}/include/interfaces
         ${CMAKE_CURRENT_LIST_DIR}/include/c-interfaces
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


