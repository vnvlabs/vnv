if(WITH_MPI)
  target_sources(injection
                 PRIVATE ${CMAKE_CURRENT_LIST_DIR}/MPICommunicator.cpp)
                 
endif()
