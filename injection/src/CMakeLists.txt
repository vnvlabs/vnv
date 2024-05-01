add_subdirectory(base)
add_subdirectory(interfaces)
add_subdirectory(c-interfaces)
add_subdirectory(plugins)
add_subdirectory(fortran-interfaces)
add_subdirectory(common-interfaces)
add_subdirectory(dist)

target_link_libraries(injection PRIVATE dl)
target_link_libraries(injection PUBLIC jsv::jsv)

target_link_libraries(injection PRIVATE Python3::Python)

set_target_properties(injection PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)
set_target_properties(injection PROPERTIES INSTALL_RPATH
                                           "${CMAKE_INSTALL_PREFIX}/lib")

target_link_libraries(injection PRIVATE inja)