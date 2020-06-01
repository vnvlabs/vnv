
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree
#find_package(Injection)

add_executable(test-runner)
target_sources(test-runner PRIVATE "test-runner.cpp")
#<needle>



configure_file(${CMAKE_CURRENT_LIST_DIR}/test-config.json ${CMAKE_CURRENT_BINARY_DIR}/test-config.json COPYONLY)
register_and_link_vnv_cpp(test-runner TestRunner ${CMAKE_CURRENT_LIST_DIR})



install(TARGETS test-runner DESTINATION test/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/test-config.json DESTINATION test/)

