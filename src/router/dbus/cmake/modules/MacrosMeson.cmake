#
# cmake package for meson support
#
# SPDX-FileCopyrightText: © 2023 Ralf Habacker
# SPDX-License-Identifier: BSD-3-Clause

#
# load meson build file into an internal list named _meson_build
#
macro(meson_init config)
    set(_meson_build_name ${config})
    file(READ ${config} _meson_build_raw)
    # Convert file contents into a CMake list (where each element in the list
    # is one line of the file)
    string(REGEX REPLACE ";" "\\\\;" _meson_build "${_meson_build_raw}")
    string(REGEX REPLACE "\n" ";" _meson_build "${_meson_build}")
endmacro()

# extracts version information from autoconf config file
# and set related cmake variables
#
# returns
#   ${prefix}_VERSION
#   ${prefix}_VERSION_STRING
#   ${prefix}_MAJOR_VERSION
#   ${prefix}_MINOR_VERSION
#   ${prefix}_MICRO_VERSION
#   ${prefix}_LIBRARY_AGE
#   ${prefix}_LIBRARY_REVISION
#   ${prefix}_LIBRARY_CURRENT
#
macro(meson_version prefix)
    set(WS "[ \t\r\n]")
    string(TOUPPER ${prefix} prefix_upper)
    string(REGEX REPLACE ".*${WS}version:${WS}*'([0-9.]+)'.*" "\\1" ${prefix_upper}_VERSION ${_meson_build_raw})
    string(REPLACE "." ";" VERSION_LIST ${${prefix_upper}_VERSION})
    list(GET VERSION_LIST 0 ${prefix_upper}_MAJOR_VERSION)
    list(GET VERSION_LIST 1 ${prefix_upper}_MINOR_VERSION)
    list(GET VERSION_LIST 2 ${prefix_upper}_MICRO_VERSION)
    set(${prefix_upper}_VERSION_STRING "${${prefix_upper}_VERSION}")
    string(REGEX REPLACE ".*${WS}lt_age${WS}*=${WS}*([0-9]+).*" "\\1" ${prefix_upper}_LIBRARY_AGE ${_meson_build_raw})
    string(REGEX REPLACE ".*${WS}lt_current${WS}*=*${WS}*([0-9]+).*" "\\1" ${prefix_upper}_LIBRARY_CURRENT ${_meson_build_raw})
    string(REGEX REPLACE ".*${WS}lt_revision${WS}*=${WS}*([0-9]+).*" "\\1" ${prefix_upper}_LIBRARY_REVISION ${_meson_build_raw})
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_VERSION          = ${${prefix_upper}_VERSION}         ")
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_VERSION_STRING   = ${${prefix_upper}_VERSION_STRING}  ")
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_MAJOR_VERSION    = ${${prefix_upper}_MAJOR_VERSION}   ")
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_MINOR_VERSION    = ${${prefix_upper}_MINOR_VERSION}   ")
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_MICRO_VERSION    = ${${prefix_upper}_MICRO_VERSION}   ")
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_LIBRARY_AGE      = ${${prefix_upper}_LIBRARY_AGE}     ")
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_LIBRARY_REVISION = ${${prefix_upper}_LIBRARY_REVISION}")
    message(STATUS "fetched variable from meson.build - ${prefix_upper}_LIBRARY_CURRENT  = ${${prefix_upper}_LIBRARY_CURRENT} ")
endmacro()
