


include(${SWIG_USE_FILE})

set_property(SOURCE IOutputReader.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE IOutputReader.i PROPERTY SWIG_MODULE_NAME VnVReader)
swig_add_library(VnVReader TYPE SHARED LANGUAGE python SOURCES IOutputReader.i)

target_include_directories(VnVReader PRIVATE ../injection/include ${PYTHON_INCLUDE_DIRS} )
set_property(TARGET VnVReader PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
target_link_libraries(VnVReader PRIVATE Injection::Injection)

