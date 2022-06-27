target_sources(
  injection
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Runtime.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Logging.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Communication.cpp
          ${CMAKE_CURRENT_LIST_DIR}/prettyfunction.cpp)