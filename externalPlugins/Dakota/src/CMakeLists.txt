target_sources(DakotaVnV PRIVATE ${CMAKE_CURRENT_LIST_DIR}/dakota_parameter_search.cpp)

target_include_directories(DakotaVnV PRIVATE ${Dakota_INCLUDE_DIRS})
target_link_libraries(DakotaVnV PRIVATE ${Dakota_LIBRARIES})
target_link_libraries(DakotaVnV PRIVATE Threads::Threads)
