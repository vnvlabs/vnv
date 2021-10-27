
// This file was create automatically by CMake.
#pragma once

#define VNVVERSIONINFOCALLBACK(PNAME)                                      \
  const char* __vnv__verison_info_##PNAME() {                              \
    return "{"                                                             \
       "\"git_retrieved_state\" : true," \
       "\"git_head_sha1\"       : \"cf6e6daf5bc4d0318a9fa23a241aa27f1f2ba1e3\"," \
       "\"git_dirty\"           : \"true," \
       "\"git_author\"          : \"ben\"," \
       "\"git_author_email\"    : \"boneill@rnet-tech.com\"," \
       "\"git_commit_date\"     : \"2021-09-30 13:23:01 -0400\"," \
       "\"git_describe\"        : \"cf6e6da\"," \
       "\"c_compiler\"          : \"/usr/bin/cc\"," \
       "\"c_compiler_version\"  : \"9.3.0\"," \
       "\"cxx_compiler\"        : \"/usr/bin/c++\"," \
       "\"cxx_compiler_version\": \"9.3.0\"," \
       "\"mpi_c_compiler\"      : \"/usr/bin/mpicc\"," \
       "\"mpi_c_version\"       : \"3.1\"," \
       "\"mpi_cxx_compiler\"    : \"/usr/bin/mpicxx\"," \
       "\"mpi_cxx_version\"     : \"3.1\"" \
       "}";\
  }
