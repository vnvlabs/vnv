cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

function(addExecutable name packageName file)
add_executable(${name} ${file})
link_vnv_file(${name} ${packageName} cpp)
install(TARGETS ${name} DESTINATION examples)
endfunction()

addExecutable(sample.a DakotaExample "sample.cpp")

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample.json
               ${CMAKE_CURRENT_BINARY_DIR}/sample.json COPYONLY)

install(
  FILES ${CMAKE_CURRENT_LIST_DIR}/sample.json
  DESTINATION examples/
)

