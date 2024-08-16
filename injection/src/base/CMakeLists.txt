add_subdirectory(stores)
add_subdirectory(parser)
add_subdirectory(communication)

target_sources(
  injection
  PRIVATE 
          ${CMAKE_CURRENT_LIST_DIR}/Runtime.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Logger.cpp
          ${CMAKE_CURRENT_LIST_DIR}/FunctionSigniture.cpp
          ${CMAKE_CURRENT_LIST_DIR}/InjectionPoint.cpp
          )
