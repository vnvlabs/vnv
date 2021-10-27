
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(psip SHARED "")

target_sources(psip PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/psip.cpp)

target_include_directories(psip PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
link_vnv_file(psip PSIP cpp)

set_target_properties(psip PROPERTIES INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/psip/lib)
target_compile_definitions(psip PRIVATE PSIP=PSIP)
target_compile_definitions(psip PRIVATE PSIPS="PSIP")

install(TARGETS psip DESTINATION examples/psip/lib)
