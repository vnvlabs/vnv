target_sources(
  injection PRIVATE
          ${CMAKE_CURRENT_LIST_DIR}/InjectionPointStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/OutputEngineStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/TestStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/TransformStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/UnitTestStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/ActionStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/OptionsParserStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/CommunicationStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/IteratorStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/PlugStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/DataTypeStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/ReductionStore.cpp
          ${CMAKE_CURRENT_LIST_DIR}/SamplerStore.cpp
          
                             
)