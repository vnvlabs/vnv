
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree
#find_package(Injection)

add_executable(test-runner)
target_sources(test-runner PRIVATE "test-runner.cpp")

#This is a bit strange. But, we need to link msgpack to
# get access to the msgpack.h file needed in the Router unit test.
# Two ways to remove this (1) change Router class to use pointers to Route and
# forward declare (remove #include SparseBitVector.h") or two, link msgpack as
# a public library. (3) leave it as is.
target_link_libraries(test-runner PRIVATE msgpack)

add_subdirectory(unit-tests)

configure_file(${CMAKE_CURRENT_LIST_DIR}/test-config.json ${CMAKE_CURRENT_BINARY_DIR}/test-config.json COPYONLY)
register_and_link_vnv_cpp(test-runner TestRunner ${PROJECT_SOURCE_DIR}/registration/)

install(TARGETS test-runner DESTINATION test/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/test-config.json DESTINATION test/)

