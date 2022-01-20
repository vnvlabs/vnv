cmake_minimum_required(VERSION 3.6)

project(vnv-docs)

find_package(Doxygen)
find_package(Python3 3.5)
find_package(VirtualEnv 15.0)

if(DOXYGEN_FOUND AND Python3_FOUND AND VirtualEnv_FOUND)

  set(BINARY_BUILD_DIR "${CMAKE_CURRENT_BINARY_DIR}/_build")
  set(SPHINX_CACHE_DIR "${CMAKE_CURRENT_BINARY_DIR}/_doctrees")
  set(SPHINX_HTML_DIR "${CMAKE_CURRENT_BINARY_DIR}/html")

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/virt/bin/sphinx-build
    COMMAND ${VIRTUALENV_EXECUTABLE} -p ${Python3_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/virt
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/virt/bin/pip install -r ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt --upgrade
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
  )

  configure_file("${CMAKE_CURRENT_LIST_DIR}/conf.py.in" "${BINARY_BUILD_DIR}/conf.py" @ONLY)

  file(GLOB_RECURSE INCLUDE_FILES ${PROJECT_SOURCE_DIR}/docs/*.rst ${PROJECT_SOURCE_DIR}/injection/include/*.h)

  add_custom_command(
    OUTPUT ${SPHINX_HTML_DIR}/index.html
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/virt/bin/sphinx-build ${INCLUDE_FILES}
    COMMAND
      ${CMAKE_CURRENT_BINARY_DIR}/virt/bin/sphinx-build -q -b html -c
      "${BINARY_BUILD_DIR}" -d "${SPHINX_CACHE_DIR}" "${CMAKE_CURRENT_LIST_DIR}"
      "${SPHINX_HTML_DIR}" 
    COMMENT "Building HTML documentation with Sphinx")


  add_custom_target(docs DEPENDS ${SPHINX_HTML_DIR}/index.html)

else()
  message(
    WARNING
      "Generating the documentation requires Python3.5+, VirtualEnv 15.0+ and Doxygen.\n Docuementation will not be built."
  )
endif()
