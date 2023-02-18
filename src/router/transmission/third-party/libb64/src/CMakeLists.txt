add_library(libb64 ${LIBB64_LIBRARY_TYPE}
    ../include/b64/cencode.h
    ../include/b64/cdecode.h
    ../include/b64/ccommon.h
    ../include/b64/encode.h
    ../include/b64/decode.h
    cdecode.c
    cencode.c)

if(NOT LIBB64_STANDALONE_BUILD)
    add_library(${PROJECT_NAME}::libb64 ALIAS libb64)
endif()

set_target_properties(libb64
    PROPERTIES
        OUTPUT_NAME b64)

target_include_directories(libb64
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(LIBB64_ENABLE_INSTALL)
    install(
        TARGETS libb64
        EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()
