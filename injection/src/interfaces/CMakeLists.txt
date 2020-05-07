add_subdirectory(nodes)

target_sources(injection PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/CppInjection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/IOutputEngine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ISerializer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ITest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ITransform.cpp
)
