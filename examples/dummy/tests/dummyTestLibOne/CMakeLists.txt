
add_library(dummyTestLibOne SHARED "")

target_sources(dummyTestLibOne PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/dummyTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EuclideanError.cpp
)

target_link_libraries(dummyTestLibOne Injection::Injection)
install(TARGETS dummyTestLibOne DESTINATION examples/dummy/test-libs)

