
find_package(Clang)

if(Clang_FOUND)

  # Here, we build the executable used to generate the code required to register
  # each of the packages.

  # We have a small chicken and egg problem in that there are a number of VnV
  # functions included in the VnV library that are also used in matcher.
  # However, we cannot fully compile the VnV library without running the matcher
  # (to generate the registration code). To get around this, we staticly link
  # source required in the matcher code into the executable. That way, we can
  # avoid requiring a linking of the VnV library to build the matcher and
  # without having to pull out a seperate utilities library.

  add_executable(
    vnv-matcher
    vv-matcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../injection/src/base/Utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../injection/src/base/exceptions.cpp)
  add_executable(Injection::Extraction ALIAS vnv-matcher)

  # Strange fruit -- Clang requires runtime info to be turned of?
  # github.com/woboq/woboq_codebrowser/issues/46 <-
  # target_compile_options(vnv-matcher PRIVATE -fno-rtti)

  target_include_directories(vnv-matcher PRIVATE ${CLANG_INCLUDE_DIRS})
  target_include_directories(
    vnv-matcher PRIVATE ${CMAKE_CURRENT_LIST_DIR}/../injection/include)

  target_compile_definitions(vnv-matcher PRIVATE -DPACKAGENAME=VnVProcessor)
  target_link_libraries(
    vnv-matcher
    PRIVATE clangAST
            clangASTMatchers
            clangBasic
            clangFrontend
            clangTooling
            clangToolingRefactoring
            jsv::jsv)

  install(TARGETS vnv-matcher DESTINATION bin)

else()
  message(WARNING "\n\n Clang Not Found. Clang tools will not be built")
endif()
