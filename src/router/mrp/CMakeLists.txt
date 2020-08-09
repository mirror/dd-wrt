# Copyright (c) 2020 Microchip Technology Inc. and its subsidiaries.
# SPDX-License-Identifier: (GPL-2.0)

project (mrp)

cmake_minimum_required(VERSION 2.6)

## libnl ################################################
find_path(LibNL_INCLUDE_DIR netlink/netlink.h
    /usr/include
    /usr/include/libnl3
    /usr/local/include
    /usr/local/include/libnl3
)

find_library(LibNL_LIBRARY NAMES nl nl-3)
find_library(LibNL_GENL_LIBRARY NAMES nl-genl nl-genl-3)

if (NOT (LibNL_INCLUDE_DIR AND LibNL_LIBRARY))
    message(FATAL_ERROR "Could not find netlink library.")
endif ()

## libmnl ###############################################
find_path(LibMNL_INCLUDE_DIR libmnl/libmnl.h
    /usr/include
    /usr/local/include
)

find_library(LibMNL_LIBRARY NAMES mnl)

if (NOT (LibMNL_INCLUDE_DIR AND LibMNL_LIBRARY))
    message(FATAL_ERROR "Coult not find mnl library.")
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DHAVE_LIBMNL")

## libev ################################################
find_path(LibEV_INCLUDE_DIR ev.h
    /usr/include
    /usr/local/include
)

find_library(LibEV_LIBRARY NAMES ev)

if (NOT (LibEV_INCLUDE_DIR AND LibEV_LIBRARY))
    message(FATAL_ERROR "Could not find libev.")
endif ()

## mrp (this project) ####################################
add_definitions(-Wall)

include_directories(${LibNL_INCLUDE_DIR} ${LibEV_INCLUDE_DIR} ${LibMNL_INCLUDE_DIR} include/uapi)

add_executable(mrp mrp.c)

add_executable(mrp_server mrp_server.c packet.c server_socket.c server_cmds.c state_machine.c offload.c timer.c libnetlink.c utils.c)
target_link_libraries(mrp_server ${LibNL_LIBRARY} ${LibNL_GENL_LIBRARY}
    ${LibEV_LIBRARY} ${LibMNL_LIBRARY})

install(TARGETS mrp_server mrp RUNTIME DESTINATION bin)

