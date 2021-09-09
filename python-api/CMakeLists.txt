
# SWIG: use standard target name.
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

# SWIG: use SWIG_MODULE_NAME property.
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

find_package(SWIG 4)
find_package(PythonLibs)
if(${SWIG_FOUND} AND ${PYTHONLIBS_FOUND})


include(${SWIG_USE_FILE})

set_property(SOURCE IOutputReader.i PROPERTY CPLUSPLUS ON)
set_property(SOURCE IOutputReader.i PROPERTY SWIG_MODULE_NAME VnVReader)
swig_add_library(
  VnVReader
  TYPE SHARED
  LANGUAGE python
  SOURCES IOutputReader.i)

# Add the python include directories and link the library. For some reason, we
# don't need to link the library on linux, but we do need to link it on mac.
#target_include_directories(VnVReader PRIVATE ../injection/include)
target_link_libraries(VnVReader PRIVATE Python3::Python)
target_compile_definitions(VnVReader PRIVATE -DVNV_IGNORE=0)
set_property(TARGET VnVReader PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
target_link_libraries(VnVReader PRIVATE Injection::Injection)


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
     
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/README.md
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


option(VNV_FORCE_PIP_USER "Force --user to be used on pip install" OFF)
option(VNV_INSTALL_READER "Install the vnv reader in detected python distribution" ON) 

if( NOT ${VNV_FORCE_PIP_USER} AND (DEFINED ENV{VIRTUAL_ENV} OR DEFINED ENV{CONDA_PREFIX}))
  set(_pip_args)
else()
  set(_pip_args "--user")
endif()

add_custom_command(TARGET VnVReader POST_BUILD COMMAND ${Python3_EXECUTABLE} ARGS -m pip install -e . ${_pip_args}
 WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)


endif()

