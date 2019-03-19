# Install script for directory: /home/seg/DEV/mpc83xx/src/router/libyang/src/extensions

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

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/lib/libyang/extensions/nacm.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/libyang/extensions/nacm.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/lib/libyang/extensions/nacm.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/libyang/extensions/nacm.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/lib/libyang/extensions" TYPE SHARED_LIBRARY FILES "/home/seg/DEV/mpc83xx/src/router/libyang/src/extensions/nacm.so")
  if(EXISTS "$ENV{DESTDIR}/usr/lib/libyang/extensions/nacm.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/libyang/extensions/nacm.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/lib/libyang/extensions/nacm.so"
         OLD_RPATH "/home/seg/DEV/mpc83xx/src/router/libyang:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/seg/DEV/mpc83xx/src/router/libyang/:" "$ENV{DESTDIR}/usr/lib/libyang/extensions/nacm.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/lib/libyang/extensions/metadata.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/libyang/extensions/metadata.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/lib/libyang/extensions/metadata.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/libyang/extensions/metadata.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/lib/libyang/extensions" TYPE SHARED_LIBRARY FILES "/home/seg/DEV/mpc83xx/src/router/libyang/src/extensions/metadata.so")
  if(EXISTS "$ENV{DESTDIR}/usr/lib/libyang/extensions/metadata.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/libyang/extensions/metadata.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/lib/libyang/extensions/metadata.so"
         OLD_RPATH "/home/seg/DEV/mpc83xx/src/router/libyang:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/seg/DEV/mpc83xx/src/router/libyang/:" "$ENV{DESTDIR}/usr/lib/libyang/extensions/metadata.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}/usr/lib/libyang/extensions/yangdata.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/libyang/extensions/yangdata.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}/usr/lib/libyang/extensions/yangdata.so"
         RPATH "")
  endif()
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/libyang/extensions/yangdata.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/lib/libyang/extensions" TYPE SHARED_LIBRARY FILES "/home/seg/DEV/mpc83xx/src/router/libyang/src/extensions/yangdata.so")
  if(EXISTS "$ENV{DESTDIR}/usr/lib/libyang/extensions/yangdata.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/libyang/extensions/yangdata.so")
    file(RPATH_CHANGE
         FILE "$ENV{DESTDIR}/usr/lib/libyang/extensions/yangdata.so"
         OLD_RPATH "/home/seg/DEV/mpc83xx/src/router/libyang:"
         NEW_RPATH "")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/seg/DEV/mpc83xx/src/router/libyang/:" "$ENV{DESTDIR}/usr/lib/libyang/extensions/yangdata.so")
    endif()
  endif()
endif()

