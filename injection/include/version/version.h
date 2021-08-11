
// This file was create automatically by CMake.
#pragma once

#define VNVVERSIONINFOCALLBACK(PNAME)                                      \
  const char* __vnv__verison_info_##PNAME() {                              \
    return "{"                                                             \
       "\"git_retrieved_state\" : true," \
       "\"git_head_sha1\"       : \"e0ff51764cd1aecdfde0ac04723493a732912733\"," \
       "\"git_dirty\"           : \"true," \
       "\"git_author\"          : \"ben\"," \
       "\"git_author_email\"    : \"boneill@rnet-tech.com\"," \
       "\"git_commit_date\"     : \"2021-08-03 13:05:00 -0400\"," \
       "\"git_describe\"        : \"e0ff517\"," \
       "\"c_compiler\"          : \"/usr/bin/cc\"," \
       "\"c_compiler_version\"  : \"10.3.0\"," \
       "\"cxx_compiler\"        : \"/usr/bin/c++\"," \
       "\"cxx_compiler_version\": \"10.3.0\"," \
       "\"mpi_c_compiler\"      : \"/usr/bin/mpicc\"," \
       "\"mpi_c_version\"       : \"3.1\"," \
       "\"mpi_cxx_compiler\"    : \"/usr/bin/mpicxx\"," \
       "\"mpi_cxx_version\"     : \"3.1\"" \
       "}";\
  }
