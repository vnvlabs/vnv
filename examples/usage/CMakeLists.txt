cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Noclang "noclang.cpp")
target_link_libraries(Noclang PRIVATE dummyLibOne Injection::Injection )

add_executable(App "app.cpp")
target_link_libraries(App PRIVATE dummyLibOne)

configure_file(${CMAKE_CURRENT_LIST_DIR}/input.json ${CMAKE_CURRENT_BINARY_DIR}/input.json COPYONLY)
