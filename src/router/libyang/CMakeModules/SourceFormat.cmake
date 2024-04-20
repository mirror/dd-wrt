# format source files with uncrustify

# check that format checking is available - always use before SOURCE_FORMAT
macro(SOURCE_FORMAT_ENABLE)
    if(NOT ${ARGC} EQUAL 1)
        message(FATAL_ERROR "source_format_enable() needs the required Uncrustify version!")
    endif()

    find_package(Uncrustify ${ARGV0})
    if(UNCRUSTIFY_FOUND)
        set(SOURCE_FORMAT_ENABLED TRUE)
    else()
        set(SOURCE_FORMAT_ENABLED FALSE)
    endif()
endmacro()

# files are expected to be a list and relative paths are resolved wtih respect to CMAKE_SOURCE DIR
macro(SOURCE_FORMAT)
    if(NOT ${ARGC})
        message(FATAL_ERROR "source_format() needs a list of files to format!")
    endif()

    if(SOURCE_FORMAT_ENABLED)
        add_custom_target(format
                COMMAND ${UNCRUSTIFY} -c ${CMAKE_SOURCE_DIR}/uncrustify.cfg --no-backup --replace ${ARGN}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Formating sources with ${UNCRUSTIFY} ...")

        add_custom_target(format-check
                COMMAND ${UNCRUSTIFY} -c ${CMAKE_SOURCE_DIR}/uncrustify.cfg --check ${ARGN}
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMENT "Checking format of the sources with ${UNCRUSTIFY} ...")

        set(SOURCE_FORMAT_ENABLED TRUE)
    endif()
endmacro()
