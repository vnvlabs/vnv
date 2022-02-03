
enable_language (Fortran)
set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/modules)
add_executable(hello_fortran helloworld.f90 vnv_fortran_mod.f90 vnv_fortran.c)

add_library(vnv_fortran vnv_fortran.c)
