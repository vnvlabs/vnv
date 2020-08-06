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
target_include_directories(VnVReader PRIVATE ../injection/include
                                             ${PYTHON_INCLUDE_DIRS})
target_link_libraries(VnVReader PRIVATE ${PYTHON_LIBRARIES})
target_compile_definitions(VnVReader PRIVATE -DVNV_IGNORE=0)
set_property(TARGET VnVReader PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
target_link_libraries(VnVReader PRIVATE Injection::Injection)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/__init__.py
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
