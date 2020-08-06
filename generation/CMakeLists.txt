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

  add_subdirectory(vnv)

  file(
    COPY ${CMAKE_CURRENT_SOURCE_DIR}/vnv-quickstart.py
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    FILE_PERMISSIONS
      OWNER_READ
      OWNER_WRITE
      OWNER_EXECUTE
      GROUP_READ
      GROUP_EXECUTE
      WORLD_READ
      WORLD_EXECUTE)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/setup.py
       DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/README.md
       DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

else()
  message("SWIG and Python Libs required to build python readers")
endif()
