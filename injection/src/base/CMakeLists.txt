add_subdirectory(stores)
add_subdirectory(parser)
add_subdirectory(points)
add_subdirectory(communication)
target_sources(
  injection
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/exceptions.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Runtime.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Utilities.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Logger.cpp
          ${CMAKE_CURRENT_LIST_DIR}/ActionType.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Provenance.cpp
          )
