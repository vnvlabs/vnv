
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree
#find_package(Injection)


add_executable(test-runner)
target_sources(test-runner PRIVATE "test-runner.cpp")
target_link_libraries(test-runner PRIVATE msgpack)
add_subdirectory(unit-tests)
register_and_link_vnv_cpp(test-runner TestRunner ${PROJECT_SOURCE_DIR}/registration/)

if (WITH_MPI)
     target_link_libraries(test-runner PRIVATE MPI::MPI_C)
     target_compile_definitions(test-runner PRIVATE -DWITH_MPI=1)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/test-config.json ${CMAKE_CURRENT_BINARY_DIR}/test-config.json COPYONLY)
install(TARGETS test-runner DESTINATION test/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/test-config.json DESTINATION test/)

