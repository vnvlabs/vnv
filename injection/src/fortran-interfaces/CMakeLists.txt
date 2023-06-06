
target_sources(
  injection
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/finterface.cpp
)

enable_language (Fortran)
set(CMAKE_Fortran_MODULE_DIRECTORY ${CMAKE_BINARY_DIR}/modules)

add_library(finjection SHARED ${CMAKE_CURRENT_LIST_DIR}/fmodule.f90)
target_link_libraries(finjection PUBLIC injection)
add_library(Injection::FInjection ALIAS finjection)

install(DIRECTORY ${CMAKE_Fortran_MODULE_DIRECTORY} DESTINATION ${CMAKE_INSTALL_PREFIX})
