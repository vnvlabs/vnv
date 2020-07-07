# SWIG: use standard target name.
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

# SWIG: use SWIG_MODULE_NAME property.
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

find_package(SWIG)
find_package(PythonLibs)
if (${SWIG_FOUND} AND ${PYTHONLIBS_FOUND})

add_subdirectory(vnv)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/example DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

else()
     message("SWIG and Python Libs required to build python readers")
endif()
