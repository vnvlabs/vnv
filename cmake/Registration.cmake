### MACRO TO BUILD THE REGISTRATION CPP FILE DYNAMICALLY
option(WITH_EXTRACTION "Use extraction to generate Registration files" ON)
set(VNV_OUT_DIR "${CMAKE_BINARY_DIR}/registration" CACHE PATH "Output Directory for Registration files")
set(VNV_OUT_PREFIX "Registration" CACHE STRING "VNV Registration file prefix")
set(VNV_OUT_EXT "c" CACHE STRING "VNV Registration Ouput file extension" )
set(VNV_COMPILE_COMMANDS "${CMAKE_BINARY_DIR}/compile_commands.json" CACHE FILEPATH "Compile Commands file")
set(VNV_DIST_PATH "${CMAKE_SOURCE_DIR}/registration/" CACHE PATH "Directory to save registraiton cache files to")


if(WITH_EXTRACTION AND TARGET Injection::Extraction)

#This target is always out of date. The dummyFile is never created, so it always runs.
#Output may or may not change


function( add_vnv_generation_target packageName filename cac deps) 
    
   if (deps) 
     set(depfiles ${cac} ${filename})
   else()
     set(depfiles ${cac})  
   endif()

   add_custom_target(
        vnv_gen_${packageName}
        COMMAND Injection::Extraction 
               --output ${filename} 
               --cache ${cac} 
               --package ${packageName} 
               {CMAKE_BINARY_DIR}/compile_commands.json
        BYPRODUCTS ${depfiles}       
   )
   
   add_custom_target(
        vnv_force_${packageName}
        COMMAND Injection::Extraction 
               --output ${filename} 
               --cache ${cac} 
               --force 
               --package ${packageName} 
               {CMAKE_BINARY_DIR}/compile_commands.json
          BYPRODUCTS ${depfiles}
               )
   
   add_custom_target(
        vnv_reset_${packageName}
        COMMAND Injection::Extraction 
               --output ${filename} 
               --cache ${cac} 
               --reset 
               --package ${packageName} 
               {CMAKE_BINARY_DIR}/compile_commands.json
          BYPRODUCTS ${depfiles}
               )

   add_custom_target(
        vnv_reset_force_${packageName}
        COMMAND Injection::Extraction 
               --output ${filename} 
               --cache ${cac} 
               --force 
               --reset
               --package ${packageName} 
               {CMAKE_BINARY_DIR}/compile_commands.json
          BYPRODUCTS ${depfiles}
   )
      
endfunction()


# This function sets the vnv targets as dependencies of some 
# list of targets (pass them in after packageName). This is usefull 
# in situations where you generate code containing injection points. 
function( add_generation_dependencies packageName ) 
   add_dependencies(vnv_gen_${packageName} ${ARGN})
   add_dependencies(vnv_force_${packageName} ${ARGN})
   add_dependencies(vnv_reset_${packageName} ${ARGN})
   add_dependencies(vnv_reset_force_${packageName} ${ARGN})

   verify_compiler_and_mpi()

endfunction()


# This one adds all the targets but makes it up to you 
function(link_vnv_file_root targetName packageName filename deps )

   #Link the vnv library
   target_link_libraries(${targetName} PRIVATE Injection::Injection)

   #Add all the custom vnv code generation targets. 
   add_vnv_generation_target( ${packageName} ${filename} ${CMAKE_BINARY_DIR}/vnv_cache.__cache__ deps )
   
endfunction()


function(link_vnv_file_man targetName packageName filename)
   link_vnv_file_root(${targetName} ${packageName} ${filename} false)
endfunction()   

# This one adds all the targets but makes it up to you 
# to add the source. 
function(link_vnv_file_semi targetName packageName filename)
   link_vnv_file_root(${targetName} ${packageName} ${filename} true)
   
   #Add the file as a source
   target_sources( ${targetName} PRIVATE ${filename} )
   
   #Ignore the generated file when processing vnv stuff. We need this because other wise
   # the parser tries to find these files which dont exist yet. 
   set_source_files_properties(
        ${filename}
        PROPERTIES COMPILE_DEFINITIONS VNV_IGNORE=0
   )

endfunction()



function(link_vnv_file targetName packageName extension)
   link_vnv_file_semi(${targetName} ${packageName} ${CMAKE_BINARY_DIR}/vnv_registration_${packageName}.${extension})
      
   # Make the main target depend on the regen step so regen happens during 
   # the build/                          
   add_dependencies(${targetName} vnv_gen_${packageName})
   
endfunction()




endif()






