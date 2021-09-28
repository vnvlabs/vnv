cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(addExecutable name packageName file)
  add_executable(${name} ${file})
  link_vnv_file(${name} ${packageName} cpp)

  if(WITH_MPI)
    target_include_directories(${name} PRIVATE ${MPI_CXX_INCLUDE_PATH})
    target_compile_options(${name} PRIVATE ${MPI_CXX_COMPILE_FLAGS})
    target_link_libraries(${name} PRIVATE ${MPI_CXX_LIBRARIES}
                                          ${MPI_CXX_LINK_FLAGS})
    target_compile_definitions(${name} PUBLIC -DWITH_MPI=1)
  endif()

  install(TARGETS ${name} DESTINATION examples/dummy/executables)
endfunction()

if(WITH_MPI)

  addexecutable(sample.a SampleExecutable "sample.cpp")
  target_link_libraries(sample.a PRIVATE dummyLibOne dummyLibTwo)
  set_target_properties(
    sample.a PROPERTIES INSTALL_RPATH
                        ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib)

  addexecutable(read.a SampleReader "read.cpp")
  addexecutable(simple.a SimpleExecutable "simple.cpp")
  addexecutable(distributed.a Distributed "distributed.cpp")
  addexecutable(live.a Live "live.cpp")
  

endif()

addexecutable(serial.a SerialExample "serial.cpp")

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample.json
               ${CMAKE_CURRENT_BINARY_DIR}/vv-input.json COPYONLY)

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample-read.json
               ${CMAKE_CURRENT_BINARY_DIR}/vv-read-input.json COPYONLY)

configure_file(${CMAKE_CURRENT_LIST_DIR}/distributed.json
               ${CMAKE_CURRENT_BINARY_DIR}/vv-dist-data.json COPYONLY)

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample-live.json
               ${CMAKE_CURRENT_BINARY_DIR}/vv-input-live.json COPYONLY)

install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/sample.json
  DESTINATION examples/dummy/executables
  RENAME vv-input.json)
