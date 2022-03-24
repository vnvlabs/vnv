# Add the subs
add_subdirectory(c)
add_subdirectory(cpp)
add_subdirectory(library)
add_subdirectory(read)
add_subdirectory(samples)
add_subdirectory(tests)
add_subdirectory(usage)

add_subdirectory(fortran)

configure_file(${CMAKE_CURRENT_LIST_DIR}/reports.json ${CMAKE_CURRENT_BINARY_DIR}/reports.json COPYONLY)
