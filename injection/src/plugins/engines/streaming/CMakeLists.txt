


target_sources( injection PRIVATE 
     ${CMAKE_CURRENT_LIST_DIR}/json_file.cpp
     ${CMAKE_CURRENT_LIST_DIR}/json_socket.cpp
     ${CMAKE_CURRENT_LIST_DIR}/json_stdout.cpp
)



find_package(CURL)
if (CURL_FOUND) 

target_link_libraries(injection PRIVATE ${CURL_LIBRARIES})
target_include_directories(injection PRIVATE ${CURL_INCLUDE_DIRS})
target_sources( injection PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}/json_http.cpp
)

endif()


option(WITH_ADIOS_ENGINE "Use the Adios Engine" ON)
if(WITH_ADIOS_ENGINE AND WITH_MPI)

  find_package(ADIOS2 REQUIRED)

  target_sources(injection PRIVATE 
        ${CMAKE_CURRENT_LIST_DIR}/adios_file.cpp
  )

  target_link_libraries(injection PRIVATE adios2::adios2)
  target_compile_definitions(injection PRIVATE WITH_ADIOS=1)

endif()



