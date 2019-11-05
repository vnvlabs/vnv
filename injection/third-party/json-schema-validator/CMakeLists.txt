

# and one for the validator
add_library(jsv SHARED 
    src/json-schema-draft7.json.cpp
    src/json-uri.cpp
    src/json-validator.cpp)

add_library(jsv::jsv ALIAS jsv)


set_target_properties(jsv
                      PROPERTIES
                      VERSION ${PROJECT_VERSION}
                      SOVERSION 1)

install(TARGETS jsv
	EXPORT jsv-targets
	LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin)


install(EXPORT jsv-targets 
	NAMESPACE
	  jsv::
	DESTINATION lib/cmake/)

install(DIRECTORY include/
        DESTINATION include
        FILES_MATCHING PATTERN "*.h*")


target_include_directories(jsv
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

target_compile_features(jsv
    PUBLIC
        cxx_range_for) # for C++11 - flags

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(jsv
        PRIVATE
            -Wall -Wextra)
endif()

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


export(EXPORT jsv-targets NAMESPACE jsv::)

export(PACKAGE jsv)


