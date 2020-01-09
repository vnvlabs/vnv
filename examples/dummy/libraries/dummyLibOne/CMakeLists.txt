
cmake_minimum_required(VERSION 3.6)                                                                           
set(CMAKE_CXX_STANDARD 11)                                                                                    
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                                                           
                                                                                                             
add_library(dummyLibOne SHARED "")

target_sources(dummyLibOne PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/class1.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/class2.cpp
)

target_compile_definitions(dummyLibOne PRIVATE -DPACKAGENAME=DummyLibOne)

target_include_directories(dummyLibOne PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)


target_link_libraries(dummyLibOne Injection::Injection)

set_target_properties(dummyLibOne PROPERTIES
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib)


install(TARGETS dummyLibOne DESTINATION examples/dummy/lib)

