add_subdirectory(base)
add_subdirectory(interfaces)
add_subdirectory(c-interfaces)
add_subdirectory(plugins)
add_subdirectory(dist)
add_subdirectory(streaming)
add_subdirectory(python)

target_link_libraries(injection PRIVATE dl)
target_link_libraries(injection PUBLIC jsv::jsv)

target_link_libraries(injection PRIVATE Python3::Python)

set_target_properties(injection PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
set_target_properties(injection PROPERTIES INSTALL_RPATH
                                           "${CMAKE_INSTALL_PREFIX}/lib")

# Message pack is used by the parallel output engine to pack buffers. This is
# listed as public to get the unit tests working.
target_link_libraries(injection PRIVATE msgpack)
target_link_libraries(injection PRIVATE inja)