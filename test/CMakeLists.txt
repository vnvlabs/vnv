
cmake_minimum_required(VERSION 3.6)                                                                           
set(CMAKE_CXX_STANDARD 11)                                                                                    
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                                                           
 
# Not needed inside the build tree
#find_package(Injection)

add_executable(test-runner "test-runner.cpp")
target_link_libraries(test-runner Injection::Injection)

install(TARGETS test-runner DESTINATION test/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/test-config.json DESTINATION test/)

