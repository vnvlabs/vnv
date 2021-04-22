# Add the adios engine to the target

if(WITH_MPI)
  find_package(MPI REQUIRED)
endif()

# Add the third party targets ( json-schema-validator TODO github-link)

add_library(injection SHARED "" include/plugins/comms/MPICommunicator.h)
link_vnv_file(injection VNV cpp)

set_target_properties(injection PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION
                                                                      1)

add_library(Injection::Injection ALIAS injection)
target_link_libraries(injection PRIVATE msgpack)

if(WITH_MPI)
  target_include_directories(
    injection
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
    PUBLIC ${MPI_CXX_INCLUDE_PATH}
           $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
           $<INSTALL_INTERFACE:include>)
  target_compile_options(injection PRIVATE ${MPI_CXX_COMPILE_FLAGS})
  target_compile_definitions(injection PRIVATE -DWITH_MPI=1)
  target_link_libraries(injection PRIVATE ${MPI_CXX_LIBRARIES}
                                          ${MPI_CXX_LINK_FLAGS})
else()
  target_include_directories(
    injection
    PRIVATE ${CMAKE_CURRENT_LIST_DIR}/include
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
           $<INSTALL_INTERFACE:include>)
endif()

add_subdirectory(src)
add_subdirectory(third-party)
add_subdirectory(util)
