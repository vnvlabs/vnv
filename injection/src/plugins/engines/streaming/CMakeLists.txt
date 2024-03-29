target_sources(
  injection
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/json_file.cpp
          ${CMAKE_CURRENT_LIST_DIR}/json_sock.cpp
          ${CMAKE_CURRENT_LIST_DIR}/json_stdout.cpp
          ${CMAKE_CURRENT_LIST_DIR}/mongo_load.cpp
          
          ${CMAKE_CURRENT_LIST_DIR}/sssssh.cpp
          
          )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")
find_package(CURL)
find_package(Microhttpd)

if(CURL_FOUND AND MICROHTTPD_FOUND)
 
  execute_process(COMMAND grep "MHD_Result" ${MICROHTTPD_INCLUDE_DIRS}/microhttpd.h RESULT_VARIABLE MHD_NO_RES OUTPUT_VARIABLE MM)
  target_compile_definitions(injection PRIVATE WITH_MHD_RESULT=${MHD_NO_RES})
  
  target_link_libraries(injection PRIVATE ${CURL_LIBRARIES})
  target_include_directories(injection PRIVATE ${CURL_INCLUDE_DIRS})
  target_include_directories(injection PRIVATE ${MICROHTTPD_INCLUDE_DIRS})
  target_link_libraries(injection PRIVATE ${MICROHTTPD_LIBRARIES})
  target_compile_definitions(injection PRIVATE WITH_LIBCURL=1)
  target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/json_http.cpp)

endif()

option(WITH_ADIOS_ENGINE "Use the Adios Engine" ON)
if(WITH_ADIOS_ENGINE AND WITH_MPI)

  find_package(ADIOS2)
  if (ADIOS2_FOUND) 
  	target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/adios_file.cpp)
	  target_link_libraries(injection PRIVATE adios2::adios2)
    target_compile_definitions(injection PRIVATE WITH_ADIOS=1)
    add_subdirectory(adios_patch)
  else()
     message(WARNING "Adios Engine Requested but ADIOS2 not found") 
  endif()	  
endif()
