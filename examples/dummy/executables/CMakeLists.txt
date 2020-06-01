
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree
#find_package(Injection)

add_executable(sample.a "sample.cpp")
register_and_link_vnv_cpp(sample.a SampleExecutable ${PROJECT_SOURCE_DIR}/registration/)
target_link_libraries(sample.a dummyLibOne dummyLibTwo)

add_executable(read.a "read.cpp")
register_and_link_vnv_cpp(read.a SampleReader ${PROJECT_SOURCE_DIR}/registration/)


target_compile_definitions(read.a PRIVATE -DPACKAGENAME=SampleReader)

if (WITH_MPI)
	target_include_directories(sample.a PRIVATE ${MPI_CXX_INCLUDE_PATH})
	target_compile_options(sample.a PRIVATE ${MPI_CXX_COMPILE_FLAGS})
	target_link_libraries(sample.a ${MPI_CXX_LIBRARIES} ${MPI_CXX_LINK_FLAGS})
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/sample.json ${CMAKE_CURRENT_BINARY_DIR}/vv-input.json COPYONLY)

# We place the test libs in a strange directory. So, for this case, we will
# set the RPATH to those libs directory. There must be a way to set the target
# up to inherit the RPATH from the linked library. But,
set_target_properties(sample.a PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib
)
install(TARGETS sample.a DESTINATION examples/dummy/executables)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/sample.json DESTINATION examples/dummy/executables RENAME vv-input.json)

set_target_properties(read.a PROPERTIES
        INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/examples/dummy/lib
)
install(TARGETS read.a DESTINATION examples/dummy/executables)
