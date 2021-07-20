# SWIG: use standard target name.
if(POLICY CMP0078)
  cmake_policy(SET CMP0078 NEW)
endif()

# SWIG: use SWIG_MODULE_NAME property.
if(POLICY CMP0086)
  cmake_policy(SET CMP0086 NEW)
endif()

find_package(SWIG 4)
if(${SWIG_FOUND})


# Run a little test to see if the python found is a virtual environemnt. This changes how we install 
# the package. 
execute_process(COMMAND ${Python3_EXECUTABLE} -c "import sys; sys.exit(sys.prefix == sys.base_prefix)" RESULT_VARIABLE virt)
if (virt EQUAL "1") 
  set(_pip_args "--user")
  message("Virtual environment not detected: --user will be used.")
else()
  set(_pip_args "")
  message("Virtual environment detected: --user will not be used.")
endif()

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
  

  install(CODE "execute_process(COMMAND ${Python3_EXECUTABLE} -m pip install -e ${CMAKE_CURRENT_BINARY_DIR} ${_pip_args} )")

else()
  message("SWIG and Python Libs required to build python report generation")
endif()
