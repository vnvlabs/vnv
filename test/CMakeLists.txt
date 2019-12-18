
cmake_minimum_required(VERSION 3.6)                                                                           
set(CMAKE_CXX_STANDARD 11)                                                                                    
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                                                           
 
# Not needed inside the build tree
#find_package(Injection)

add_executable(test-runner "test-runner.cpp"  "unit-testers/parser.cpp")
configure_file(${CMAKE_CURRENT_LIST_DIR}/test-config.json ${CMAKE_CURRENT_BINARY_DIR}/test-config.json COPYONLY)
target_link_libraries(test-runner Injection::Injection)
target_compile_definitions(test-runner PRIVATE -DPACKAGENAME=TestRunner)
install(TARGETS test-runner DESTINATION test/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/test-config.json DESTINATION test/)

