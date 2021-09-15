target_sources(
  injection
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/breakpoint.cpp
          ${CMAKE_CURRENT_LIST_DIR}/cputime.cpp
          ${CMAKE_CURRENT_LIST_DIR}/vectorDump.cpp
          ${CMAKE_CURRENT_LIST_DIR}/continue.cpp
          ${CMAKE_CURRENT_LIST_DIR}/printf.cpp)
