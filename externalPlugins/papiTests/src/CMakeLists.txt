target_sources(papiTest PRIVATE ${CMAKE_CURRENT_LIST_DIR}/hardwareInfo.cpp)

target_include_directories(papiTest PRIVATE ${PAPI_INCLUDE_DIRS})
target_link_libraries(papiTest PRIVATE ${PAPI_LIBRARIES})
