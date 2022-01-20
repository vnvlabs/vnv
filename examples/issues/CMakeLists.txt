
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(issues SHARED "")

target_sources(issues PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/issues.cpp)

target_include_directories(issues PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
link_vnv_file(issues ISSUES cpp)

set_target_properties(issues PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/issues/lib)
target_compile_definitions(issues PRIVATE ISSUES=ISSUES)
