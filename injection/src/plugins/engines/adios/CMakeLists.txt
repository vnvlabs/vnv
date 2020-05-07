
option(WITH_ADIOS_ENGINE "Use the Adios Engine" ON)

if(WITH_ADIOS_ENGINE)
  find_package(ADIOS2)
  if (ADIOS2_FOUND)
    target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/AdiosEngineManager.cpp)
    target_link_libraries(injection PRIVATE adios2::adios2 )
    target_compile_definitions(injection PRIVATE WITH_MPI=1 PRIVATE WITH_ADIOS=1)
  endif()
endif()
