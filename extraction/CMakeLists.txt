option(WITH_EXTRACTION "WITH CLANG_TOOL EXTRACTION" ON)
if(NOT LLVM_DIR)
  message("Please Set LLVM_DIR to the LLVM version to use.")
endif()

set(search_paths
    ${LLVM_DIR} ${LLVM_DIR}/lib/cmake ${LLVM_DIR}/lib/cmake/llvm
    ${LLVM_DIR}/lib/cmake/clang ${LLVM_DIR}/share/clang/cmake/
    ${LLVM_DIR}/share/llvm/cmake/)

find_package(LLVM CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)
find_package(Clang CONFIG PATHS ${search_paths} NO_DEFAULT_PATH)

if(WITH_EXTRACTION AND NOT Clang_FOUND)
  message(
    FATAL_ERROR
      "Extraction requested but clang not found. Please
  set the LLVM_DIR using -DLLVM_DIR=<path-to-llvm>. The toolkit requires
  clang version 10 or higher.

  If you do not have clang, or you do not want to use clang, pass the parameter
  -DWITH_EXTRACTION=OFF. Clang is required to autVo generate the registration.cpp file.
 The VnV Library will be built using the stored registration file. Without Clang,
 all registration must be completed manually.")
endif()

if(NOT WITH_EXTRACTION)
  message(WARNING "Compiling with automatic registration turned off")
endif()

if(Clang_FOUND)
  if(${LLVM_VERSION} VERSION_LESS 10)
    message(FATAL_ERROR "Toolkit requires Clang Version 10 or higher")
    set(Clang_FOUND OFF)
  elseif(${LLVM_VERSION} VERSION_GREATER_EQUAL 13)
    message(WARNING "Toolkit has not been tested with versions after 12")
  endif()
endif()

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
    vv-matcher.cpp  Printer.cpp Writer.cpp Preprocessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../injection/src/base/Utilities.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../injection/src/base/exceptions.cpp)
  add_executable(Injection::Extraction ALIAS vnv-matcher)

  target_include_directories(vnv-matcher SYSTEM PRIVATE ${CLANG_INCLUDE_DIRS})
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

  install(
    TARGETS vnv-matcher
    EXPORT injection-targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
  set_target_properties(vnv-matcher PROPERTIES EXPORT_NAME Extraction)
endif()
