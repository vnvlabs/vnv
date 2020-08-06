### MACRO TO BUILD THE REGISTRATION CPP FILE DYNAMICALLY
option(WITH_EXTRACTION "Use extraction to generate Registration files" ON)
set(VNV_OUT_DIR "${CMAKE_BINARY_DIR}/registration" CACHE PATH "Output Directory for Registration files")
set(VNV_OUT_PREFIX "Registration" CACHE STRING "VNV Registration file prefix")
set(VNV_OUT_EXT "c" CACHE STRING "VNV Registration Ouput file extension" )
set(VNV_COMPILE_COMMANDS "${CMAKE_BINARY_DIR}/compile_commands.json" CACHE FILEPATH "Compile Commands file")
set(VNV_DIST_PATH "${CMAKE_SOURCE_DIR}/registration/" CACHE PATH "Directory to save registraiton cache files to")


if(WITH_EXTRACTION AND TARGET Injection::Extraction)

add_custom_target(vnv-register
   COMMAND mkdir ARGS -p ${VNV_OUT_DIR}
   COMMAND Injection::Extraction ARGS --output ${VNV_OUT_DIR}/${VNV_OUT_PREFIX} --nowrite ${CMAKE_BINARY_DIR}/compile_commands.json

)

function(link_vnv_file targetName packageName extension)

add_custom_command(
     OUTPUT ${VNV_OUT_DIR}/${VNV_OUT_PREFIX}_${packageName}.${extension}
     COMMAND test -e ${VNV_OUT_DIR} || ${CMAKE_COMMAND} -E make_directory ${VNV_OUT_DIR}
     COMMAND Injection::Extraction ARGS --package ${packageName} --output ${VNV_OUT_DIR}/${VNV_OUT_PREFIX} --extension ${extension} ${CMAKE_BINARY_DIR}/compile_commands.json
     COMMAND cp ${VNV_OUT_DIR}/${VNV_OUT_PREFIX}_${packageName}.${extension} ${VNV_DIST_PATH}
     DEPENDS vnv-register
)

target_sources(${targetName} PRIVATE ${VNV_OUT_DIR}/${VNV_OUT_PREFIX}_${packageName}.${extension})
set_source_files_properties(${VNV_OUT_DIR}/${VNV_OUT_PREFIX}_${packageName}.${extension} PROPERTIES COMPILE_DEFINITIONS VNV_IGNORE=0)
target_link_libraries(${targetName} PRIVATE Injection::Injection)

endfunction()

add_custom_target(vnv-register-clean
     COMMAND rm ${VNV_OUT_DIR}/${VNV_OUT_PREFIX}.__cache__
     COMMAND rm ${VNV_OUT_DIR}/${VNV_OUT_PREFIX}*.${VNV_OUT_EXT}
)

else()
# No extraction to be used. Hope that the distpath has a up to date copy.
function(link_vnv_file targetName packageName extension)

add_custom_command(
     OUTPUT ${VNV_OUT_DIR}/${VNV_OUT_PREFIX}_${packageName}.${extension}
     COMMAND test -e ${VNV_OUT_DIR} || ${CMAKE_COMMAND} -E make_directory ${VNV_OUT_DIR}
     COMMAND cp ${VNV_DIST_PATH}/${VNV_OUT_PREFIX}_${packageName}.${extension} ${VNV_OUT_DIR}
)
target_sources(${targetName} PRIVATE ${VNV_OUT_DIR}/${VNV_OUT_PREFIX}_${packageName}.${extension})
set_source_files_properties(${VNV_OUT_DIR}/${VNV_OUT_PREFIX}_${packageName}.${extension} PROPERTIES COMPILE_DEFINITIONS VNV_IGNORE=0)
target_link_libraries(${targetName} PRIVATE Injection::Injection)

endfunction()


endif()
