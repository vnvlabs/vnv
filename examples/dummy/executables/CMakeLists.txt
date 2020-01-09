
cmake_minimum_required(VERSION 3.6)                                                                           
set(CMAKE_CXX_STANDARD 11)                                                                                    
set(CMAKE_CXX_STANDARD_REQUIRED ON)                                                                           
 
# Not needed inside the build tree
#find_package(Injection)

add_executable(sample.a "sample.cpp")
target_link_libraries(sample.a dummyLibOne dummyLibTwo Injection::Injection)

target_compile_definitions(sample.a PRIVATE -DPACKAGENAME=SampleExecutable)

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample.json ${CMAKE_CURRENT_BINARY_DIR}/sample.json COPYONLY)

# We place the test libs in a strange directory. So, for this case, we will 
# set the RPATH to those libs directory. There must be a way to set the target
# up to inherit the RPATH from the linked library. But, 
set_target_properties(sample.a PROPERTIES 
	INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib
)

install(TARGETS sample.a DESTINATION examples/dummy/executables)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/sample.json DESTINATION examples/dummy/executables)

