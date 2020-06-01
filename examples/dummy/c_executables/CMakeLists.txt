
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree
#find_package(Injection)

add_executable(csample.a "sample.c")
register_and_link_vnv_c(csample.a CSampleExecutable ${CMAKE_CURRENT_LIST_DIR}/registration/)

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample.json ${CMAKE_CURRENT_BINARY_DIR}/sample.json COPYONLY)

install(TARGETS csample.a DESTINATION examples/dummy/c_executables)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/sample.json DESTINATION examples/dummy/c_executables)

