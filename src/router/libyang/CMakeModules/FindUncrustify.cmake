# - Find uncrustify
# Find the uncrustify binary.
#
# UNCRUSTIFY         - path ot the binary
# UNCRUSTIFY_VERSION - found version
# UNCRUSTIFY_FOUND   - True if uncrustify found.
include(FindPackageHandleStandardArgs)

find_program(UNCRUSTIFY uncrustify)
if(UNCRUSTIFY)
    execute_process(COMMAND ${UNCRUSTIFY} --version OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE VERSION)
    string(FIND ${VERSION} "-" START_IDX)
    math(EXPR START_IDX "${START_IDX} + 1")
    string(SUBSTRING "${VERSION}" ${START_IDX} -1 VERSION)

    string(FIND ${VERSION} "-" LEN)
    string(SUBSTRING "${VERSION}" 0 ${LEN} UNCRUSTIFY_VERSION)
endif()

# Handle the QUIETLY and REQUIRED arguments and set UNCRUSTIFY_FOUND to TRUE if all listed variables are TRUE.
find_package_handle_standard_args(Uncrustify REQUIRED_VARS UNCRUSTIFY VERSION_VAR UNCRUSTIFY_VERSION)
