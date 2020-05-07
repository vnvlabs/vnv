add_subdirectory(base)
add_subdirectory(interfaces)
add_subdirectory(c-interfaces)
add_subdirectory(plugins)
add_subdirectory(dist)
add_subdirectory(python)


#Add the Base files to the injection target
target_sources(injection PRIVATE
     ${CMAKE_CURRENT_LIST_DIR}/Registration.cpp
)

target_link_libraries(injection PRIVATE dl)
target_link_libraries(injection PUBLIC jsv::jsv)


