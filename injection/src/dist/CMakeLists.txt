
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  target_sources(injection
                 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/linux/DistUtils.cpp)
else()
  message(FATAL_ERROR "Dist is not MACOS or Linux")
endif()

