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
target_include_directories(yangre BEFORE PRIVATE ${PROJECT_BINARY_DIR})
