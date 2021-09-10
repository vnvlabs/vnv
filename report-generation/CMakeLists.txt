add_subdirectory(python_api)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/app
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/run.py
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

     

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/requirements.txt
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

find_package(VirtualEnv 15.0 REQUIRED )
find_package(Python3 3.5 REQUIRED)

execute_process(
     COMMAND ${VIRTUALENV_EXECUTABLE} -p ${Python3_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/virt
)
execute_process(
     COMMAND ${CMAKE_CURRENT_BINARY_DIR}/virt/bin/pip3 install -r ${CMAKE_CURRENT_BINARY_DIR}/requirements.txt         
)
