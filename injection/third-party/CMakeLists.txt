

set(BUILD_SHARED_LIBS ON CACHE BOOL "Turn on shared libraries" FORCE)
set(FMT_INSTALL ON CACHE BOOL "Turn on install target" FORCE)
add_subdirectory(fmt)
add_subdirectory(json-schema-validator)
