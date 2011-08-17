cmake_minimum_required(VERSION 2.6)

PROJECT(rpc C)
ADD_DEFINITIONS(-Os -Wall --std=gnu99 -g3 -I. -include compat.h)

FILE(GLOB SOURCES *.c)

ADD_LIBRARY(rpc SHARED ${SOURCES})

SET(CMAKE_INSTALL_PREFIX /usr)

FILE(GLOB headers rpc/*.h)
INSTALL(FILES ${headers}
	DESTINATION include/rpc
)
INSTALL(TARGETS rpc
	LIBRARY DESTINATION lib
)
