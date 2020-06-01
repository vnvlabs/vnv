
add_library(dummyTestLibOne SHARED "")

target_sources(dummyTestLibOne PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/dummyTest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/EuclideanError.cpp
)


##Generate the VNV Registration code
register_and_link_vnv_cpp(dummyTestLibOne DummyTestLibOne ${PROJECT_SOURCE_DIR}/registration/)
install(TARGETS dummyTestLibOne DESTINATION examples/dummy/test-libs)

