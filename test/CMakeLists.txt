cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree find_package(Injection)

add_executable(test-runner)
target_sources(test-runner PRIVATE "test-runner.cpp")
add_subdirectory(unit-tests)
link_vnv_executable(test-runner VnVTestRunner cpp)

if(WITH_MPI)
  target_link_libraries(test-runner PRIVATE MPI::MPI_CXX)
  target_compile_definitions(test-runner PRIVATE -DWITH_MPI=1)
endif()
target_include_directories(test-runner PRIVATE ${CMAKE_BINARY_DIR}/build_includes)


configure_file(${CMAKE_CURRENT_LIST_DIR}/test-config.json
               ${CMAKE_CURRENT_BINARY_DIR}/test-config.json COPYONLY)
install(TARGETS test-runner DESTINATION test/)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/test-config.json DESTINATION test/)
