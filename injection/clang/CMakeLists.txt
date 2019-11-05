

find_package(Clang)

add_executable(vnv-processor
  VnVProcessor.cpp
)

# Strange fruit -- Clang requires runtime info to be turned of? 
# github.com/woboq/woboq_codebrowser/issues/46 <- 
target_compile_options(vnv-processor PRIVATE -fno-rtti)

target_link_libraries(vnv-processor PRIVATE  
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  )

target_include_directories(vnv-processor PRIVATE ${CLANG_INCLUDE_DIRS})

install(TARGETS vnv-processor DESTINATION bin)

add_executable(vnv-matcher
  vv-matcher.cpp
)

# Strange fruit -- Clang requires runtime info to be turned of? 
# github.com/woboq/woboq_codebrowser/issues/46 <- 
target_compile_options(vnv-matcher PRIVATE -fno-rtti)

target_include_directories(vnv-matcher PRIVATE ${CLANG_INCLUDE_DIRS})

target_link_libraries(vnv-matcher PRIVATE  
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  jsv::jsv
  )


install(TARGETS vnv-matcher DESTINATION bin)
