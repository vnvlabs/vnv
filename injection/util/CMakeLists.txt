
set(INCLUDE_COMPILER_STRING "-I ${CMAKE_CURRENT_LIST_DIR}/../include -I ${CMAKE_CURRENT_LIST_DIR}/../third-party/json-schema-validator/include")

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/macros.info
  COMMAND
  ${CMAKE_CXX_COMPILER} -E -dM -I${CMAKE_CURRENT_LIST_DIR}/../include -I${CMAKE_CURRENT_LIST_DIR}/../third-party/json-schema-validator/include
    ${CMAKE_CURRENT_LIST_DIR}/../include/VnV.h >
    ${CMAKE_CURRENT_BINARY_DIR}/macros.info
)

add_custom_target(macro_tracker ALL
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/macros.info)
file(
  COPY ${CMAKE_CURRENT_LIST_DIR}/subpackage.py
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
  FILE_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE)
