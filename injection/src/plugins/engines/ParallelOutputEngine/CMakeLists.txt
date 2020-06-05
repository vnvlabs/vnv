#if (WITH_MPI)

target_sources(injection PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/ParallelOutputEngineImpl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/Router.cpp
        )

#endif()
