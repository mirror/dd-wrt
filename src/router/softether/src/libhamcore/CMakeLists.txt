cmake_minimum_required(VERSION 3.11)

project(libhamcore LANGUAGES C)

set(CMAKE_C_STANDARD 99)

include(TestBigEndian)

add_library(libhamcore STATIC)

set_property(TARGET libhamcore PROPERTY POSITION_INDEPENDENT_CODE ON)

test_big_endian(BIG_ENDIAN)
if(BIG_ENDIAN)
  target_compile_definitions(libhamcore PRIVATE "BYTE_ORDER_BIG_ENDIAN")
endif()

if(MSVC)
  # Suppress "warning C4996: 'fopen': This function or variable may be unsafe. Consider using fopen_s instead.".
  # fopen_s() is part of C11, but we want to stick with C99 for increased portability.
  target_compile_definitions(libhamcore PRIVATE "_CRT_SECURE_NO_WARNINGS")
endif()

target_include_directories(libhamcore PUBLIC "include")

target_sources(libhamcore
  PRIVATE
    FileSystem.c
    FileSystem.h
    Hamcore.c
    Memory.c
    Memory.h
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Hamcore.h"
)

find_package(ZLIB REQUIRED)
target_link_libraries(libhamcore PRIVATE ZLIB::ZLIB)
