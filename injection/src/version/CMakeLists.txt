# Define the two required variables before including the source code for
# watching a git repository.
set(PRE_CONFIGURE_FILE
    "${CMAKE_SOURCE_DIR}/injection/include/version/version.h.in")
set(POST_CONFIGURE_FILE
    "${CMAKE_SOURCE_DIR}/injection/include/version/version.h")
include(${CMAKE_CURRENT_LIST_DIR}/cmake/git_watcher.cmake)
target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/version.cpp)
add_dependencies(injection check_git)
