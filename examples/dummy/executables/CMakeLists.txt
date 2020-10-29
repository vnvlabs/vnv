cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree find_package(Injection)

add_executable(sample.a "sample.cpp")
target_link_libraries(sample.a PRIVATE dummyLibOne dummyLibTwo)
link_vnv_file(sample.a SampleExecutable cpp)

add_executable(read.a "read.cpp")
link_vnv_file(read.a SampleReader cpp)

if(WITH_MPI)
  target_include_directories(sample.a PRIVATE ${MPI_CXX_INCLUDE_PATH})
  target_compile_options(sample.a PRIVATE ${MPI_CXX_COMPILE_FLAGS})
  target_link_libraries(sample.a PRIVATE ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})
  target_compile_definitions(sample.a PUBLIC -DWITH_MPI=1)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample.json
               ${CMAKE_CURRENT_BINARY_DIR}/vv-input.json COPYONLY)

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample_with_papi.json
      ${CMAKE_CURRENT_BINARY_DIR}/vv-input-papi.json COPYONLY )

# We place the test libs in a strange directory. So, for this case, we will set
# the RPATH to those libs directory. There must be a way to set the target up to
# inherit the RPATH from the linked library. But,
set_target_properties(
  sample.a PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib)
install(TARGETS sample.a DESTINATION examples/dummy/executables)
install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/sample.json
  DESTINATION examples/dummy/executables
  RENAME vv-input.json)

set_target_properties(
  read.a PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib)
install(TARGETS read.a DESTINATION examples/dummy/executables)
