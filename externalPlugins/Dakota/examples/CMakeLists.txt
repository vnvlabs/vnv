cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(MPI REQUIRED)

function(addExecutable name packageName file)

add_executable(${name} ${file})
link_vnv_file(${name} ${packageName} cpp)

target_include_directories(${name} PRIVATE ${MPI_CXX_INCLUDE_PATH})
target_compile_options(${name} PRIVATE ${MPI_CXX_COMPILE_FLAGS})
target_link_libraries(${name} PRIVATE ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})
target_compile_definitions(${name} PUBLIC -DWITH_MPI=1)

install(TARGETS ${name} DESTINATION examples)

endfunction()

addExecutable(sample.a DakotaExample "sample.cpp")

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample.json
               ${CMAKE_CURRENT_BINARY_DIR}/sample.json COPYONLY)

install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/sample.json
  DESTINATION examples/
)

