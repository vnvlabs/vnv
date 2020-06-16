### MACRO TO BUILD THE REGISTRATION CPP FILE DYNAMICALLY
option(WITH_EXTRACTION "Use extraction to generate Registration files" ON)

option(REGENERATE_ALL_REGISTRATION "Regenerate all registration objects" OFF)

if (REGENERATE_ALL_REGISTRATION)
  set(VNV_REGEN --useCache )
endif()

function(generate_vnv_registration targetName packageName distPath extension)

  if(WITH_EXTRACTION AND TARGET Injection::Extraction)
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
        COMMAND Injection::Extraction ARGS ${VNV_REGEN} --package ${packageName} --output ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${CMAKE_BINARY_DIR}/compile_commands.json
        COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${distPath}
        COMMENT "Running VnV Registration Detection for ${packageName}"
        BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension} ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension}.__cache__
      )
    add_dependencies(${targetName} vnv_${targetName}_generation)
  else()

      ## Copy the saved dist and use that. Rerun this command whenever that changes.
      add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension}
          COMMAND cp ARGS ${distPath}/Registration_${packageName}.${extension} ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension}
          DEPENDS ${distPath}/Registration_${packageName}.${extension}
      )
  endif()

  #Add the target.
  target_sources(${targetName} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/Registration_${packageName}.${extension})

endfunction()

function(register_and_link_vnv targetName packageName distPath extension)
     generate_vnv_registration(${targetName} ${packageName} ${distPath} ${extension} )
     target_compile_definitions(${targetName} PRIVATE -DPACKAGENAME=${packageName} )
     target_link_libraries(${targetName} PRIVATE Injection::Injection)
endfunction()

function(register_and_link_vnv_c targetName packageName distPath)
     register_and_link_vnv(${targetName} ${packageName} ${distPath} c )
endfunction()

function(register_and_link_vnv_cpp targetName packageName distPath)
     register_and_link_vnv(${targetName} ${packageName} ${distPath} cpp )
endfunction()
