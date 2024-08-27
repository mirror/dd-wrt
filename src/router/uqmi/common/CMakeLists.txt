
SET(COMMON_SOURCES qmi-message.c mbim.c utils.c)

ADD_LIBRARY(common ${COMMON_SOURCES})
ADD_DEPENDENCIES(common gen-headers gen-errors)
TARGET_INCLUDE_DIRECTORIES(common PRIVATE ${ubox_include_dir} ${blobmsg_json_include_dir} ${json_include_dir} ${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR})
