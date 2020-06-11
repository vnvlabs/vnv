

add_library(jsv SHARED
    src/json-schema-draft7.json.cpp
    src/json-uri.cpp
    src/json-validator.cpp)

add_library(jsv::jsv ALIAS jsv)

target_include_directories(jsv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(jsv
        PRIVATE
            -DJSON_SCHEMA_VALIDATOR_EXPORTS)
endif()
# regex with boost if gcc < 4.9 - default is std::regex
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9.0")
        find_package(Boost COMPONENTS regex)
        if(NOT Boost_FOUND)
            message(STATUS "GCC less then 4.9 and boost-regex NOT found - no regex used")
            target_compile_definitions(jsv PRIVATE -DJSON_SCHEMA_NO_REGEX)
        else()
            message(STATUS "GCC less then 4.9 and boost-regex FOUND - using boost::regex")
            target_compile_definitions(jsv PRIVATE -DJSON_SCHEMA_BOOST_REGEX)
            target_include_directories(jsv PRIVATE ${Boost_INCLUDE_DIRS})
            target_link_libraries(jsv PRIVATE ${Boost_LIBRARIES})
        endif()
    endif()
endif()

install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")

include(GNUInstallDirs)
set(INSTALL_JSV_CONFIGDIR ${CMAKE_INSTALL_LIBDIR}/cmake/jsv)
# and one for the validator
install(TARGETS jsv
        EXPORT jsv-targets
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
       )

set_target_properties(jsv PROPERTIES EXPORT_NAME jsv)

install(EXPORT jsv-targets
  FILE
    jsvTargets.cmake
  NAMESPACE
    jsv::
  DESTINATION lib/cmake/jsv
)

#Create a ConfigVersion.cmake file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/jsvConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(${CMAKE_CURRENT_LIST_DIR}/cmake/jsvConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/jsvConfig.cmake
    INSTALL_DESTINATION ${INSTALL_JSV_CONFIGDIR}
)

#Install the config, configversion and custom find modules
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/jsvConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/jsvConfigVersion.cmake
  DESTINATION ${INSTALL_JSV_CONFIGDIR}
)

export(EXPORT jsv-targets FILE ${CMAKE_CURRENT_BINARY_DIR}/jsvTargets.cmake NAMESPACE jsv::)
export(PACKAGE jsv)




