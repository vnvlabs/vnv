
cmake_minimum_required(VERSION 3.6)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Not needed inside the build tree
#find_package(Injection)

add_executable(newtonSolve "ged.cpp" "nlnewt.cpp" "main.cpp")
target_include_directories(newtonSolve PRIVATE ${CMAKE_CURRENT_LIST_DIR}/)
register_and_link_vnv_cpp(newtonSolve NewtonSolver ${PROJECT_SOURCE_DIR}/registration/)

if (WITH_MPI)
        target_link_libraries(newtonSolve PRIVATE MPI::MPI_C)
endif()

configure_file(${CMAKE_CURRENT_LIST_DIR}/newton-input.json ${CMAKE_CURRENT_BINARY_DIR}/newton-input.json COPYONLY)
