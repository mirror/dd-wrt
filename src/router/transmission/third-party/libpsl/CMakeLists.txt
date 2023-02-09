# Note: this isn't a full CMakeLists.txt for this project.
# This is just the minimum subset needed for building an embedded
# static library into Transmission.

cmake_minimum_required(VERSION 3.0)

project(psl
  VERSION 0.21.1.0 # when changing this, must set LIBPSL_VERSION_NUMBER too
  LANGUAGES C
)
set(LIBPSL_VERSION_NUMBER 0x001501)
set(LIBPSL_VERSION_MAJOR ${psl_VERSION_MAJOR})
set(LIBPSL_VERSION_MINOR ${psl_VERSION_MINOR})
set(LIBPSL_VERSION_PATCH ${psl_VERSION_PATCH})
set(LIBPSL_VERSION "${psl_VERSION_MAJOR}.${psl_VERSION_MINOR}.${psl_VERSION_PATCH}")

add_definitions(
  -DPACKAGE_VERSION=\"${LIBPSL_VERSION}\"
  -DBUILDING_PSL=1
  -DPSL_STATIC=1

  # ensure that builtin list is enabled even though the client code
  # is going to ensure utf8 + lowercase instead of bundling more
  # dependencies here (e.g. libidn2, libunistring)
  -DFORCE_BUILTIN_LIST=1
)

set(PSL_FILE "${CMAKE_SOURCE_DIR}/list/public_suffix_list.dat")

find_program(
  PYTHON python
  NAMES python2 python3 python3.7
  HINTS /usr/pkg/bin
  REQUIRED
)
add_custom_target(
  "suffixes_dafsa.h"
  ALL
  "${PYTHON}"
  "${CMAKE_SOURCE_DIR}/src/psl-make-dafsa"
  --output-format=cxx+
  "${PSL_FILE}"
  "${CMAKE_BINARY_DIR}/suffixes_dafsa.h"
)

configure_file(
  "${CMAKE_SOURCE_DIR}/include/libpsl.h.in"
  "${CMAKE_BINARY_DIR}/libpsl.h"
)

add_library(${PROJECT_NAME} STATIC
  src/lookup_string_in_fixed_set.c
  src/psl.c
)

include_directories(
  "${CMAKE_BINARY_DIR}"
)

add_dependencies(
  "${PROJECT_NAME}"
  "suffixes_dafsa.h"
)

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(FILES "${CMAKE_BINARY_DIR}/libpsl.h" DESTINATION include)
