
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(dummyLibOne SHARED "")

target_sources(dummyLibOne PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/class1.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/class2.cpp
)


target_include_directories(dummyLibOne PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

register_and_link_vnv_cpp(dummyLibOne DummyLibOne ${CMAKE_CURRENT_LIST_DIR})


set_target_properties(dummyLibOne PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib)

install(TARGETS dummyLibOne DESTINATION examples/dummy/lib)

