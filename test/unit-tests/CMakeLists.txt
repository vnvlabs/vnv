function(add_private_source target filename)
  target_sources(${target} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/${filename})
endfunction()

add_private_source(test-runner parser.cpp)
add_private_source(test-runner comm.cpp)
