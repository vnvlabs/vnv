target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/Communication.cpp
                                 ${CMAKE_CURRENT_LIST_DIR}/CommMapper.cpp
                                 ${CMAKE_CURRENT_LIST_DIR}/ICommunicator.cpp
                                 
                                 )

target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/MPICommunicator.cpp)
                                 

