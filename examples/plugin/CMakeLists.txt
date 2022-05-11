add_library(vnvplugin)
link_vnv_file(vnvplugin VnVPlugin cpp)

target_sources(
  vnvplugin
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/CustomActions.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomDataTypes.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomIterator.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomOptions.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomPlug.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomReduction.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomTest.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomTransform.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomUnitTests.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CustomSampler.cpp)
        
install(TARGETS vnvplugin DESTINATION examples/plugin)
