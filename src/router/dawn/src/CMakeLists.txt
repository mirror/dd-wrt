cmake_minimum_required(VERSION 2.6)
PROJECT(dawn)

INCLUDE_DIRECTORIES(include)

ADD_DEFINITIONS(-Os -Wall -Werror --std=gnu99 -Wmissing-declarations -Wno-unknown-warning-option -Wno-format-truncation)

SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")

SET(SOURCES
        main.c

        storage/datastorage.c
        include/datastorage.h

        network/networksocket.c
        include/networksocket.h

        network/broadcastsocket.c
        include/broadcastsocket.h

        network/multicastsocket.c
        include/multicastsocket.h

        utils/ubus.c
        include/ubus.h

        utils/msghandler.c
        include/msghandler.h

        utils/dawn_uci.c
        include/dawn_uci.h

        crypto/crypto.c
        include/crypto.h

        include/utils.h
        utils/utils.c

        utils/mac_utils.c
        include/mac_utils.h

        utils/memory_utils.c
        include/memory_utils.h

        include/tcpsocket.h
        network/tcpsocket.c

        include/dawn_iwinfo.h
        utils/dawn_iwinfo.c

        utils/ieee80211_utils.c
        include/ieee80211_utils.h)

SET(SOURCES_TEST_STORAGE
        test/test_storage.c
        include/test_storage.h

        include/ubus.h

        utils/utils.c
        include/utils.h

        utils/mac_utils.c
        include/mac_utils.h

        utils/memory_utils.c
        include/memory_utils.h

        storage/datastorage.c
        include/datastorage.h

        utils/ieee80211_utils.c
        include/ieee80211_utils.h)

SET(SOURCES_TEST_HEADER
        test/test_header.c)

SET(LIBS
        ubox ubus json-c blobmsg_json uci gcrypt iwinfo pthread)

ADD_EXECUTABLE(dawn ${SOURCES})
ADD_EXECUTABLE(test_storage ${SOURCES_TEST_STORAGE})
ADD_EXECUTABLE(test_header ${SOURCES_TEST_HEADER})

TARGET_LINK_LIBRARIES(dawn ${LIBS})

INSTALL(TARGETS dawn
        RUNTIME DESTINATION /usr/sbin/)
