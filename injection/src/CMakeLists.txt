add_subdirectory(base)
add_subdirectory(interfaces)
add_subdirectory(c-interfaces)
add_subdirectory(plugins)
add_subdirectory(dist)
add_subdirectory(python)

target_link_libraries(injection PRIVATE dl)
target_link_libraries(injection PUBLIC jsv::jsv)


