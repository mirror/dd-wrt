# Install script for directory: /home/seg/DEV/mpc83xx/src/router/libyang

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "0")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/xfs/toolchains/toolchain-powerpc_e300c3_gcc-13.1.0_musl/bin/powerpc-linux-uclibc-objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/yang/modules/libyang/")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/yang/modules/libyang" TYPE DIRECTORY FILES "/home/seg/DEV/mpc83xx/src/router/libyang/models/" FILES_MATCHING REGEX "/[^/]*\\.yang$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3.0.8"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES
    "/home/seg/DEV/mpc83xx/src/router/libyang/build/libyang.so.3.0.8"
    "/home/seg/DEV/mpc83xx/src/router/libyang/build/libyang.so.3"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3.0.8"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so.3"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHANGE
           FILE "${file}"
           OLD_RPATH "/home/seg/DEV/mpc83xx/src/router/_staging/usr/lib:"
           NEW_RPATH "")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/home/seg/DEV/mpc83xx/src/router/libyang/build/:" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE SHARED_LIBRARY FILES "/home/seg/DEV/mpc83xx/src/router/libyang/build/libyang.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so"
         OLD_RPATH "/home/seg/DEV/mpc83xx/src/router/_staging/usr/lib:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/seg/DEV/mpc83xx/src/router/libyang/build/:" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/libyang.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/libyang" TYPE FILE FILES
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/context.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/hash_table.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/dict.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/in.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/libyang.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/log.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/out.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/parser_data.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/parser_schema.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/plugins.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/plugins_exts.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/plugins_exts/metadata.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/plugins_types.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/printer_data.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/printer_schema.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/set.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/tree.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/tree_data.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/tree_edit.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/src/tree_schema.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/build/src/version.h"
    "/home/seg/DEV/mpc83xx/src/router/libyang/build/src/ly_config.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE FILES "/home/seg/DEV/mpc83xx/src/router/libyang/build/libyang.pc")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for the subdirectory.
  include("/home/seg/DEV/mpc83xx/src/router/libyang/build/tools/cmake_install.cmake")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/seg/DEV/mpc83xx/src/router/libyang/build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
