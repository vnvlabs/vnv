target_sources(
  injection
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/IOutputEngine.cpp
          ${CMAKE_CURRENT_LIST_DIR}/ITest.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Initialization.cpp
          ${CMAKE_CURRENT_LIST_DIR}/IReduction.cpp
          ${CMAKE_CURRENT_LIST_DIR}/IDataType.cpp
          ${CMAKE_CURRENT_LIST_DIR}/IOptions.cpp
          ${CMAKE_CURRENT_LIST_DIR}/ISampler.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Injection.cpp
       )
