
cmake_minimum_required(VERSION 3.6)                                                                           
set(CMAKE_CXX_STANDARD 11)                                                                                    
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                                                           
                                                                                                             

add_library(dummyLibTwo SHARED "")


target_sources(dummyLibTwo PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/class1.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/class2.cpp
)

target_compile_definitions(dummyLibTwo PRIVATE -DPACKAGENAME=DummyLibTwo)

target_include_directories(dummyLibTwo PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)
target_link_libraries(dummyLibTwo Injection::Injection)

set_target_properties(dummyLibTwo PROPERTIES 
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib)

install(TARGETS dummyLibTwo DESTINATION examples/dummy/lib)


