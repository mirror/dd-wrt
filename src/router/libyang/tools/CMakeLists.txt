# config file for tools
configure_file(${PROJECT_SOURCE_DIR}/tools/config.h.in ${PROJECT_BINARY_DIR}/tools/config.h @ONLY)

if(WIN32)
    find_library(GETOPT_LIBRARY NAMES getopt REQUIRED)
    find_path(GETOPT_INCLUDE_DIR NAMES getopt.h REQUIRED)
    message(STATUS "Found <getopt.h> at ${GETOPT_INCLUDE_DIR}, library at ${GETOPT_LIBRARY}")
endif()

add_subdirectory(lint)
add_subdirectory(re)

set(format_sources
    ${format_sources}
    PARENT_SCOPE)
