cmake_minimum_required(VERSION 2.6)

PROJECT(rpc C)
ADD_DEFINITIONS(-Os -Wall --std=gnu99 -I. -include rpc/compat.h)

FILE(GLOB SOURCES *.c)

ADD_LIBRARY(rpc STATIC ${SOURCES})

SET(CMAKE_INSTALL_PREFIX /usr)

FILE(GLOB headers rpc/*.h)
INSTALL(FILES ${headers}
	DESTINATION include/rpc
)
INSTALL(TARGETS rpc
	DESTINATION lib
)
