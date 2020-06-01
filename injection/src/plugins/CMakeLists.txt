add_subdirectory(tests)
add_subdirectory(engines)
add_subdirectory(unitTests)
add_subdirectory(serializers)
add_subdirectory(transforms)

#Add the Base files to the injection target
#target_sources(injection PRIVATE
#     ${CMAKE_CURRENT_LIST_DIR}/Registration.cpp
#)
