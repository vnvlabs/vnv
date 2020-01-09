

find_package(Clang)

if (Clang_FOUND)

add_executable(vnv-matcher
  vv-matcher.cpp
)
add_executable(vnv-plugin
    vv-processor.cpp
)

# Strange fruit -- Clang requires runtime info to be turned of? 
# github.com/woboq/woboq_codebrowser/issues/46 <- 
target_compile_options(vnv-matcher PRIVATE -fno-rtti)
target_compile_options(vnv-plugin PRIVATE -fno-rtti)

target_include_directories(vnv-matcher PRIVATE ${CLANG_INCLUDE_DIRS})
target_include_directories(vnv-plugin PRIVATE ${CLANG_INCLUDE_DIRS})

target_link_libraries(vnv-matcher PRIVATE  
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  jsv::jsv
)

target_link_libraries(vnv-plugin PRIVATE
  clangAST
  clangASTMatchers
  clangBasic
  clangFrontend
  clangTooling
  clangToolingRefactoring
  jsv::jsv
)


install(TARGETS vnv-matcher DESTINATION bin)
install(TARGETS vnv-plugin DESTINATION bin)

else()
	message(WARNING "\n\n Clang Not Found. Clang tools will not be built" ) 
endif()

