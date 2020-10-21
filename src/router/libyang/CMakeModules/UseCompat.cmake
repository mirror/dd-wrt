cmake_minimum_required(VERSION 2.8.12)

include(CheckSymbolExists)
include(TestBigEndian)

macro(USE_COMPAT)
    # compatibility checks
    set(CMAKE_REQUIRED_DEFINITIONS -D_POSIX_C_SOURCE=200809L)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D__BSD_VISIBLE=1)
    check_symbol_exists(vdprintf "stdio.h;stdarg.h" HAVE_VDPRINTF)
    check_symbol_exists(asprintf "stdio.h" HAVE_ASPRINTF)
    check_symbol_exists(vasprintf "stdio.h" HAVE_VASPRINTF)

    check_symbol_exists(get_current_dir_name "unistd.h" HAVE_GET_CURRENT_DIR_NAME)

    check_symbol_exists(strndup "string.h" HAVE_STRNDUP)
    check_symbol_exists(getline "stdio.h" HAVE_GETLINE)

    TEST_BIG_ENDIAN(IS_BIG_ENDIAN)

    # header and object file
    configure_file(${PROJECT_SOURCE_DIR}/compat/compat.h.in ${PROJECT_BINARY_DIR}/compat.h @ONLY)
    include_directories(${PROJECT_BINARY_DIR})
    add_library(compat OBJECT ${PROJECT_SOURCE_DIR}/compat/compat.c)
    set_property(TARGET compat PROPERTY POSITION_INDEPENDENT_CODE ON)
endmacro()
