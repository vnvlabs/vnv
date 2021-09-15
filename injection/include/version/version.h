
// This file was create automatically by CMake.
#pragma once

#define VNVVERSIONINFOCALLBACK(PNAME)                                 \
  const char* __vnv__verison_info_##PNAME() {                         \
    return "{"                                                        \
           "\"git_retrieved_state\" : true,"                          \
           "\"git_head_sha1\"       : "                               \
           "\"7ee23e6b7cdd492d595a75dc8ca5ff7cfec2eaa5\","            \
           "\"git_dirty\"           : \"true,"                        \
           "\"git_author\"          : \"ben\","                       \
           "\"git_author_email\"    : \"boneill@rnet-tech.com\","     \
           "\"git_commit_date\"     : \"2021-09-10 14:51:46 -0400\"," \
           "\"git_describe\"        : \"7ee23e6\","                   \
           "\"c_compiler\"          : \"/usr/bin/cc\","               \
           "\"c_compiler_version\"  : \"10.3.0\","                    \
           "\"cxx_compiler\"        : \"/usr/bin/c++\","              \
           "\"cxx_compiler_version\": \"10.3.0\","                    \
           "\"mpi_c_compiler\"      : \"/usr/bin/mpicc\","            \
           "\"mpi_c_version\"       : \"3.1\","                       \
           "\"mpi_cxx_compiler\"    : \"/usr/bin/mpicxx\","           \
           "\"mpi_cxx_version\"     : \"3.1\""                        \
           "}";                                                       \
  }
