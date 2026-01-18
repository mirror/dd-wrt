project(wide-integer)

cmake_minimum_required(VERSION 3.16.3)

if (${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_SOURCE_DIR})
  set(WIDE_INTEGER_MASTER_PROJECT ON)
else()
  set(WIDE_INTEGER_MASTER_PROJECT OFF)
endif()

option(WIDE_INTEGER_ENABLE_TESTS "Enable/disable tests" ${WIDE_INTEGER_MASTER_PROJECT})

if (WIDE_INTEGER_ENABLE_TESTS)
  find_package(Boost)
  if (Boost_FOUND)
    include(CTest)

    add_subdirectory("examples")
    add_subdirectory("test")
  endif()
endif()

add_library(WideInteger INTERFACE)
target_compile_features(WideInteger INTERFACE cxx_std_14)

target_include_directories(
  WideInteger SYSTEM INTERFACE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include>)

add_library(WideInteger::WideInteger ALIAS WideInteger)

install(TARGETS WideInteger EXPORT WideIntegerTargets)
install(
  FILES math/wide_integer/uintwide_t.h
  DESTINATION include/math/wide_integer/)
install(EXPORT WideIntegerTargets
  FILE WideIntegerConfig.cmake
  NAMESPACE WideInteger::
  DESTINATION lib/cmake/wide-integer)
