target_sources(injection PRIVATE
      ${CMAKE_CURRENT_LIST_DIR}/InjectionPoint.cpp
      ${CMAKE_CURRENT_LIST_DIR}/InjectionPointStore.cpp
      ${CMAKE_CURRENT_LIST_DIR}/OutputEngineStore.cpp
      ${CMAKE_CURRENT_LIST_DIR}/exceptions.cpp
      ${CMAKE_CURRENT_LIST_DIR}/JsonParser.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Runtime.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Utilities.cpp
      ${CMAKE_CURRENT_LIST_DIR}/TestStore.cpp
      ${CMAKE_CURRENT_LIST_DIR}/TransformStore.cpp
      ${CMAKE_CURRENT_LIST_DIR}/UnitTestStore.cpp
      ${CMAKE_CURRENT_LIST_DIR}/SerializerStore.cpp
      ${CMAKE_CURRENT_LIST_DIR}/JsonSchema.cpp
      ${CMAKE_CURRENT_LIST_DIR}/OptionsParserStore.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Logger.cpp
      ${CMAKE_CURRENT_LIST_DIR}/Communication.cpp
      ${CMAKE_CURRENT_LIST_DIR}/CommunicationStore.cpp
)
