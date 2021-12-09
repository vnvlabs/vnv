target_sources(
  injection
  PRIVATE ${CMAKE_CURRENT_LIST_DIR}/IDN.cpp
          ${CMAKE_CURRENT_LIST_DIR}/Nodes.cpp
          
          )


find_package (mongoc-1.0 1.7)
if (mongoc-1.0_FOUND) 
  target_compile_definitions(injection PRIVATE MONGO_DB=1)
  target_sources(injection PRIVATE ${CMAKE_CURRENT_LIST_DIR}/MongoNodeImpl.cpp ${CMAKE_CURRENT_LIST_DIR}/dispatch.cpp)
  
         
  target_link_libraries (injection PRIVATE mongo::mongoc_shared)
else()
  target_compile_definitions(injection PRIVATE MONGO_DB=0)
endif()
