
cmake_minimum_required(VERSION 3.6)

project(vnv LANGUAGES CXX C)

set(PROJECT_VERSION 1.0.0)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)


if(POLICY CMP0079)
  cmake_policy(SET CMP0079 NEW)
endif()

option(WITH_MPI "Build using MPI" OFF)

if (WITH_MPI)
        find_package(MPI REQUIRED)
endif()

if (NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "No build type selected, default to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type (default RelWitDebInfo)")
endif()

option(BUILD_EXAMPLES "Build the examples" ON)

### MACRO TO BUILD THE REGISTRATION CPP FILE DYNAMICALLY
function(generate_vnv_registration targetName packageName distPath extension)

  find_package(Clang)
  if(Clang_FOUND)
      #Use the VnV Matcher to generate the Cpp files.
      #TODO -- Set dependencies so this happens automatically whenever the files change
      #TODO -- Add a "cache" to make it faster -- only parse files that have changed.
      # Touch command is used to avoid errors where the generated file is included in the compile commands file,
      # but does not yet exist.
      file(TOUCH ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension})
      #add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension}
      #  COMMAND vnv-matcher ARGS --package ${packageName} --output ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${CMAKE_BINARY_DIR}/compile_commands.json
     #   COMMAND cp ARGS ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${distPath}/Registration_${packageName}.${extension}
     #   DEPENDS vnv-matcher
     #)
     add_custom_target(vnv_${targetName}_generation
        COMMAND vnv-matcher ARGS --package ${packageName} --output ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${CMAKE_BINARY_DIR}/compile_commands.json
        COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${distPath}
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension}.__cache__
      )
    add_dependencies(${targetName} vnv_${targetName}_generation)
  else()

      message(WARNING "Clang is required to auto generate the registration.cpp file.
       The VnV Library will be built using the stored registration file. Without Clang,
       all registration must be completed manually."
      )
      ## Copy the saved dist and use that. Rerun this command whenever that changes.
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension}
          COMMAND cp ARGS ${distPath}/Registration_${packageName}.{extension} ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension}
          DEPENDS ${distPath}/Registration_{packageName}.{extension}
      )
  endif()

  #Add the target.
  target_sources(${targetName} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension})

endfunction()



function(register_and_link_vnv targetName packageName distPath extension)
     generate_vnv_registration(${targetName} ${packageName} ${distPath} ${extension} )
     target_compile_definitions(${targetName} PRIVATE -DPACKAGENAME=${packageName} )
     target_link_libraries(${targetName} Injection::Injection)
endfunction()

function(register_and_link_vnv_c targetName packageName distPath)
     register_and_link_vnv(${targetName} ${packageName} ${distPath} c )
endfunction()

function(register_and_link_vnv_cpp targetName packageName distPath)
     register_and_link_vnv(${targetName} ${packageName} ${distPath} cpp )
endfunction()

add_subdirectory(extraction)
add_subdirectory(injection)
add_subdirectory(generation)

option(BUILD_TESTS "Build the test runner" ON)
if (BUILD_TESTS)
  add_subdirectory(test)
endif()

if (BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
add_subdirectory(docs)
