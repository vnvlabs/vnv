

# SWIG: use standard target name.
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

# SWIG: use SWIG_MODULE_NAME property.
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})
find_package(PythonLibs)

set_property(SOURCE IOutputReader.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE IOutputReader.i PROPERTY SWIG_MODULE_NAME VnVReader)
swig_add_library(VnVReader TYPE SHARED LANGUAGE python SOURCES IOutputReader.i)

target_include_directories(VnVReader PRIVATE ../include ${PYTHON_INCLUDE_DIRS} )
set_property(TARGET VnVReader PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
target_link_libraries(VnVReader PRIVATE Injection::Injection)

