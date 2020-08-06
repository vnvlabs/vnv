add_library(dummyTestLibOne SHARED "")

target_sources(
  dummyTestLibOne PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/dummyTest.cpp
                          ${CMAKE_CURRENT_LIST_DIR}/src/EuclideanError.cpp)

# Generate the VNV Registration code
link_vnv_file(dummyTestLibOne DummyTestLibOne cpp)

install(TARGETS dummyTestLibOne DESTINATION examples/dummy/test-libs)
