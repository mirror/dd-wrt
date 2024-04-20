# yangre

set(resrc
    main.c)

set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    PARENT_SCOPE)

add_executable(yangre ${resrc} ${compatsrc})
target_link_libraries(yangre yang)
install(TARGETS yangre DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_SOURCE_DIR}/tools/re/yangre.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
target_include_directories(yangre BEFORE PRIVATE ${PROJECT_BINARY_DIR})

if(WIN32)
    target_include_directories(yangre PRIVATE ${GETOPT_INCLUDE_DIR})
    target_link_libraries(yangre ${GETOPT_LIBRARY})
endif()
