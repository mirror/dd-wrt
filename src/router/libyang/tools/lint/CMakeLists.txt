# yanglint
set(lintsrc
    main_ni.c
    cmd.c
    cmd_add.c
    cmd_clear.c
    cmd_data.c
    cmd_list.c
    cmd_feature.c
    cmd_load.c
    cmd_print.c
    cmd_searchpath.c
    cmd_extdata.c
    cmd_help.c
    cmd_verb.c
    cmd_debug.c
    yl_opt.c
    yl_schema_features.c
    common.c
)

if(ENABLE_YANGLINT_INTERACTIVE)
    set(lintsrc ${lintsrc}
        main.c
        completion.c
        configuration.c
        linenoise/linenoise.c)
else()
    set(lintsrc ${lintsrc}
        main_ni_only.c)
endif()

set(format_sources
    ${format_sources}
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    PARENT_SCOPE)

add_executable(yanglint ${lintsrc} ${compatsrc})
target_link_libraries(yanglint yang)
install(TARGETS yanglint DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${PROJECT_SOURCE_DIR}/tools/lint/yanglint.1 DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
target_include_directories(yanglint BEFORE PRIVATE ${PROJECT_BINARY_DIR})

if(WIN32)
    target_include_directories(yanglint PRIVATE ${GETOPT_INCLUDE_DIR})
    target_link_libraries(yanglint ${GETOPT_LIBRARY})
endif()
