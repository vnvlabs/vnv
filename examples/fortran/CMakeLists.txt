
enable_language (Fortran)
set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/modules)
add_executable(hello_fortran helloworld.f90 reg.f90 reg.c )

target_link_libraries(hello_fortran finjection)

