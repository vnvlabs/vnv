
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
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/InjectionPoint.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/InjectionPointStore.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/OutputEngineStore.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/JsonParser.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/Runtime.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/Utilities.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/TestStore.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/TransformStore.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/UnitTestStore.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/transformer.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/SerializerStore.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/JsonSchema.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/OptionsParserStore.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/base/Logger.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/c-interfaces/Runtime.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/c-interfaces/CJson.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/c-interfaces/Logging.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/c-interfaces/Injection.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/c-interfaces/CppInjection.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/interfaces/IOutputEngine.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/interfaces/ISerializer.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/interfaces/ITest.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/interfaces/ITransform.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/Registration.cpp
                         ${CMAKE_CURRENT_LIST_DIR}/src/plugins/engines/DebugOutputEngineImpl.cpp)

if (APPLE)
    target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/dist/macos/DistUtils.cpp)
elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
    target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/dist/linux/DistUtils.cpp)
else()
    message(FATAL_ERROR "Dist is not MACOS or Linux")
endif()

target_link_libraries(injection PRIVATE dl)
target_link_libraries(injection PUBLIC jsv::jsv)
target_link_libraries(injection PRIVATE xxhash::xxhash)


if ( WITH_BUILT_IN_TESTS)
target_sources(injection PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/src/plugins/tests/provenance.cpp)
endif()



target_include_directories(injection PRIVATE
                            ${CMAKE_CURRENT_LIST_DIR}/include
                            PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
                            $<INSTALL_INTERFACE:include>
                          )

target_compile_definitions(injection PRIVATE -DPACKAGENAME=VnV Framework)

if(WITH_ADIOS_ENGINE)
find_package(ADIOS2)
if (ADIOS2_FOUND) 
  target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/plugins/engines/AdiosOutputEngineImpl.cpp)
  target_link_libraries(injection PRIVATE adios2::adios2 )
  target_compile_definitions(injection PRIVATE WITH_MPI=1 PRIVATE WITH_ADIOS=1)
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


