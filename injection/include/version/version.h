
// This file was create automatically by CMake.
#pragma once

#define VNVVERSIONINFOCALLBACK(PNAME)                                      \
  const char* __vnv__verison_info_##PNAME() {                              \
    return "{"                                                             \
       "\"git_retrieved_state\" : true," \
       "\"git_head_sha1\"       : \"b677d8089003ef00418f54a22b025b2ef52fe497\"," \
       "\"git_dirty\"           : \"true," \
       "\"git_author\"          : \"ben\"," \
       "\"git_author_email\"    : \"boneill@rnet-tech.com\"," \
       "\"git_commit_date\"     : \"2021-10-22 13:07:16 -0400\"," \
       "\"git_describe\"        : \"b677d80\"," \
       "\"c_compiler\"          : \"/bin/x86_64-linux-gnu-gcc-10\"," \
       "\"c_compiler_version\"  : \"10.3.0\"," \
       "\"cxx_compiler\"        : \"/bin/x86_64-linux-gnu-g++-10\"," \
       "\"cxx_compiler_version\": \"10.3.0\"," \
       "\"mpi_c_compiler\"      : \"/usr/bin/mpicc\"," \
       "\"mpi_c_version\"       : \"3.1\"," \
       "\"mpi_cxx_compiler\"    : \"/usr/bin/mpicxx\"," \
       "\"mpi_cxx_version\"     : \"3.1\"" \
       "}";\
  }
