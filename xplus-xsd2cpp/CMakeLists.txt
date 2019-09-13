cmake_minimum_required(VERSION 2.8.5)

project (xplus-xsd)

include(ExternalProject)


###Build the external xplus-cpp
ExternalProject_Add(libxsd2cpp
 SOURCE_DIR        ${CMAKE_CURRENT_SOURCE_DIR}
 CONFIGURE_COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/autogen.sh COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/configure --prefix ${CMAKE_INSTALL_PREFIX}/xplus-xsd2cpp
 BUILD_COMMAND ${MAKE}
)
