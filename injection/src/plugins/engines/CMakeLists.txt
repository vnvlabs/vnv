target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/DebugOutputEngineImpl.cpp)

if (WITH_ADIOS_ENGINE)
    cmake_policy (SET CMP0079 NEW) # to allow adding link library to injection target
	find_package(ADIOS2)
	if (ADIOS2_FOUND) 
	  target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/AdiosOutputEngineImpl.cpp)
	  target_link_libraries(injection PRIVATE adios2::adios2 )
	  target_compile_definitions(injection PRIVATE WITH_ADIOS=1)
	endif()
endif()

add_subdirectory(ParallelOutputEngine)
