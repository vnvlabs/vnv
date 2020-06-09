add_subdirectory(base)
add_subdirectory(interfaces)
add_subdirectory(c-interfaces)
add_subdirectory(plugins)
add_subdirectory(dist)
add_subdirectory(python)

target_link_libraries(injection PRIVATE dl)
target_link_libraries(injection PUBLIC jsv::jsv)

## Message pack is used by the parallel output engine to pack buffers.
## This is listed as public to get the unit tests working.
target_link_libraries(injection PRIVATE msgpack)



