cmake_minimum_required(VERSION 2.6)

IF (BUILD_EXAMPLES)
    PROJECT(ubox-examples C)
    ADD_DEFINITIONS(-O1 -Wall -Werror --std=gnu99 -g3)

    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)
    LINK_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/..)

    FIND_LIBRARY(json NAMES json-c json)

    ADD_EXECUTABLE(ustream-example ustream-example.c)
    TARGET_LINK_LIBRARIES(ustream-example ubox)

    ADD_EXECUTABLE(json_script-example json_script-example.c)
    TARGET_LINK_LIBRARIES(json_script-example ubox blobmsg_json json_script ${json})
ENDIF()
