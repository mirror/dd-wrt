# config file for tools
configure_file(${PROJECT_SOURCE_DIR}/tools/config.h.in ${PROJECT_BINARY_DIR}/tools/config.h @ONLY)

add_subdirectory(lint)
add_subdirectory(re)

set(format_sources
    ${format_sources}
    PARENT_SCOPE)
