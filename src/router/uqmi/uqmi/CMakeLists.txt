

SET(UQMI uqmi.c dev.c commands.c ${SOURCES})

ADD_EXECUTABLE(uqmi ${UQMI})
ADD_DEPENDENCIES(uqmi gen-headers gen-errors)

TARGET_LINK_LIBRARIES(uqmi ${LIBS} common qmigen)
TARGET_INCLUDE_DIRECTORIES(uqmi PRIVATE ${ubox_include_dir} ${blobmsg_json_include_dir} ${json_include_dir} ${CMAKE_SOURCE_DIR})

INSTALL(TARGETS uqmi
	RUNTIME DESTINATION sbin
)
