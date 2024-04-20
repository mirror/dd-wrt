if(WIN32)
    cmake_minimum_required(VERSION 3.22.0)
else()
    cmake_minimum_required(VERSION 2.8.12...3.28.1)
endif()

# force out-of-source build
if(${CMAKE_CURRENT_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_BINARY_DIR})
    message(FATAL_ERROR "In-source build is not allowed. Please make a standalone build directory and run CMake from there. You may need to remove CMakeCache.txt.")
endif()

project(libyang C)

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/")

include(GNUInstallDirs)
include(UseCompat)
include(ABICheck)
include(SourceFormat)
include(GenDoc)
include(GenCoverage)

# set default build type if not specified by user
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()
# normalize build type string
# see https://github.com/CESNET/libyang/pull/1692 for why CMAKE_C_FLAGS_<type> are not used directly
string(TOUPPER "${CMAKE_BUILD_TYPE}" BUILD_TYPE_UPPER)
if ("${BUILD_TYPE_UPPER}" STREQUAL "RELEASE")
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build Type" FORCE)
    set(CMAKE_C_FLAGS "-DNDEBUG -O2 ${CMAKE_C_FLAGS}")
elseif("${BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type" FORCE)
    set(CMAKE_C_FLAGS "-g3 -O0 ${CMAKE_C_FLAGS}")
elseif("${BUILD_TYPE_UPPER}" STREQUAL "RELWITHDEBINFO")
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build Type" FORCE)
elseif("${BUILD_TYPE_UPPER}" STREQUAL "RELWITHDEBUG")
    set(CMAKE_BUILD_TYPE "RelWithDebug" CACHE STRING "Build Type" FORCE)
elseif("${BUILD_TYPE_UPPER}" STREQUAL "ABICHECK")
    set(CMAKE_BUILD_TYPE "ABICheck" CACHE STRING "Build Type" FORCE)
    set(CMAKE_C_FLAGS "-g -Og ${CMAKE_C_FLAGS}")
elseif("${BUILD_TYPE_UPPER}" STREQUAL "DOCONLY")
    set(CMAKE_BUILD_TYPE "DocOnly" CACHE STRING "Build Type" FORCE)
endif()

#
# variables
#

set(LIBYANG_DESCRIPTION "libyang is YANG data modelling language parser and toolkit written (and providing API) in C.")

# Correct RPATH usage on OS X
set(CMAKE_MACOSX_RPATH TRUE)

# keep all binaries in the build directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# set version of the project
set(LIBYANG_MAJOR_VERSION 2)
set(LIBYANG_MINOR_VERSION 2)
set(LIBYANG_MICRO_VERSION 8)
set(LIBYANG_VERSION ${LIBYANG_MAJOR_VERSION}.${LIBYANG_MINOR_VERSION}.${LIBYANG_MICRO_VERSION})
# set version of the library
set(LIBYANG_MAJOR_SOVERSION 3)
set(LIBYANG_MINOR_SOVERSION 0)
set(LIBYANG_MICRO_SOVERSION 8)
set(LIBYANG_SOVERSION_FULL ${LIBYANG_MAJOR_SOVERSION}.${LIBYANG_MINOR_SOVERSION}.${LIBYANG_MICRO_SOVERSION})
set(LIBYANG_SOVERSION ${LIBYANG_MAJOR_SOVERSION})

if(WIN32)
    set(C_STANDARD 11)
    set(C_STANDARD_REQUIRED ON)
    set(CMAKE_C_FLAGS "/Zc:preprocessor /W3 /wd4711 /w14013 /utf-8 ${CMAKE_C_FLAGS}")
else()
    # global C flags
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -std=c11")
endif()

include_directories(${PROJECT_BINARY_DIR}/src ${PROJECT_SOURCE_DIR}/src ${PROJECT_SOURCE_DIR}/src/plugins_exts)

# type plugins are separate because they have their documentation generated
set(type_plugins
    src/plugins_types/binary.c
    src/plugins_types/bits.c
    src/plugins_types/boolean.c
    src/plugins_types/decimal64.c
    src/plugins_types/empty.c
    src/plugins_types/enumeration.c
    src/plugins_types/identityref.c
    src/plugins_types/instanceid.c
    src/plugins_types/instanceid_keys.c
    src/plugins_types/integer.c
    src/plugins_types/leafref.c
    src/plugins_types/lyds_tree.c
    src/plugins_types/string.c
    src/plugins_types/union.c
    src/plugins_types/ipv4_address.c
    src/plugins_types/ipv4_address_no_zone.c
    src/plugins_types/ipv6_address.c
    src/plugins_types/ipv6_address_no_zone.c
    src/plugins_types/ipv4_prefix.c
    src/plugins_types/ipv6_prefix.c
    src/plugins_types/date_and_time.c
    src/plugins_types/hex_string.c
    src/plugins_types/xpath1.0.c
    src/plugins_types/node_instanceid.c)

set(libsrc
    src/ly_common.c
    src/log.c
    src/hash_table.c
    src/dict.c
    src/set.c
    src/path.c
    src/diff.c
    src/context.c
    src/json.c
    src/tree_data.c
    src/tree_data_free.c
    src/tree_data_common.c
    src/tree_data_hash.c
    src/tree_data_new.c
    src/parser_xml.c
    src/parser_json.c
    src/parser_lyb.c
    src/out.c
    src/printer_data.c
    src/printer_xml.c
    src/printer_json.c
    src/printer_lyb.c
    src/schema_compile.c
    src/schema_compile_node.c
    src/schema_compile_amend.c
    src/schema_features.c
    src/tree_data_sorted.c
    src/tree_schema.c
    src/tree_schema_free.c
    src/tree_schema_common.c
    src/in.c
    src/lyb.c
    src/parser_common.c
    src/parser_yang.c
    src/parser_yin.c
    src/printer_schema.c
    src/printer_yang.c
    src/printer_yin.c
    src/printer_tree.c
    src/plugins.c
    src/plugins_types.c
    src/plugins_exts.c
    src/plugins_exts/metadata.c
    src/plugins_exts/nacm.c
    src/plugins_exts/yangdata.c
    src/plugins_exts/schema_mount.c
    src/plugins_exts/structure.c
    src/xml.c
    src/xpath.c
    src/validation.c
    ${type_plugins})

set(headers
    src/context.h
    src/hash_table.h
    src/dict.h
    src/in.h
    src/libyang.h
    src/log.h
    src/out.h
    src/parser_data.h
    src/parser_schema.h
    src/plugins.h
    src/plugins_exts.h
    src/plugins_exts/metadata.h
    src/plugins_types.h
    src/printer_data.h
    src/printer_schema.h
    src/set.h
    src/tree.h
    src/tree_data.h
    src/tree_edit.h
    src/tree_schema.h)

set(internal_headers
    src/ly_common.h
    src/diff.h
    src/hash_table_internal.h
    src/in_internal.h
    src/json.h
    src/lyb.h
    src/out_internal.h
    src/parser_internal.h
    src/path.h
    src/plugins_internal.h
    src/printer_internal.h
    src/schema_compile.h
    src/schema_compile_amend.h
    src/schema_compile_node.h
    src/schema_features.h
    src/tree_data_internal.h
    src/tree_schema_internal.h
    src/validation.h
    src/xml.h
    src/xpath.h)

set(gen_headers
    src/version.h
    src/ly_config.h)

# files to generate doxygen from
set(doxy_files
    doc/build.dox
    doc/transition_1_2.dox
    doc/transition_2_3.dox
    ${headers}
    ${PROJECT_BINARY_DIR}/src/version.h
    ${type_plugins})

# project (doxygen) logo
set(project_logo
    doc/logo.png)

# source files to be covered by the 'format' target
set(format_sources
    compat/*.c
    compat/*.h*
    src/*.c
    src/*.h
    src/plugins_exts/*
    src/plugins_types/*)
#
# options
#

if(("${BUILD_TYPE_UPPER}" STREQUAL "DEBUG") OR ("${BUILD_TYPE_UPPER}" STREQUAL "RELWITHDEBINFO"))
    option(ENABLE_TESTS "Build tests" ON)
    option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" ON)
else()
    option(ENABLE_TESTS "Build tests" OFF)
    option(ENABLE_VALGRIND_TESTS "Build tests with valgrind" OFF)
endif()
option(ENABLE_PERF_TESTS "Build performance tests" OFF)
option(ENABLE_COVERAGE "Build code coverage report from tests" OFF)
option(ENABLE_FUZZ_TARGETS "Build target programs suitable for fuzzing with AFL" OFF)
option(ENABLE_INTERNAL_DOCS "Generate doxygen documentation also from internal headers" OFF)
option(ENABLE_YANGLINT_INTERACTIVE "Enable interactive CLI yanglint" ON)
option(ENABLE_TOOLS "Build binary tools 'yanglint' and 'yangre'" ON)
option(BUILD_SHARED_LIBS "By default, shared libs are enabled. Turn off for a static build." ON)
set(YANG_MODULE_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/yang/modules/libyang" CACHE STRING "Directory where to copy the YANG modules to")

if(ENABLE_INTERNAL_DOCS)
    set(doxy_files ${doxy_files} ${internal_headers})
    set(INTERNAL_DOCS YES)
else()
    set(INTERNAL_DOCS NO)
endif()

set(LYD_VALUE_SIZE "24" CACHE STRING "Maximum size in bytes of data node values that do not need to be allocated dynamically, minimum is 8")
if(LYD_VALUE_SIZE LESS 8)
    message(FATAL_ERROR "Data node value size \"${LYD_VALUE_SIZE}\" is not valid.")
endif()
set(PLUGINS_DIR "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/libyang" CACHE STRING "Directory with libyang plugins (extensions and user types)")
set(PLUGINS_DIR_EXTENSIONS "${PLUGINS_DIR}/extensions" CACHE STRING "Directory with libyang user extensions plugins")
set(PLUGINS_DIR_TYPES "${PLUGINS_DIR}/types" CACHE STRING "Directory with libyang user types plugins")

#
# checks
#
if(NOT BUILD_SHARED_LIBS)
    message(STATUS "Disabling tests for static build")
    set(ENABLE_TESTS OFF)
    set(ENABLE_VALGRIND_TESTS OFF)
endif()

if(ENABLE_VALGRIND_TESTS)
    if(NOT ENABLE_TESTS)
        message(WARNING "Tests are disabled! Disabling memory leak tests.")
        set(ENABLE_VALGRIND_TESTS OFF)
    else()
        find_program(VALGRIND_FOUND valgrind)
        if(NOT VALGRIND_FOUND)
            message(WARNING "valgrind executable not found! Disabling memory leak tests.")
            set(ENABLE_VALGRIND_TESTS OFF)
        endif()
    endif()
endif()

if(ENABLE_TESTS)
    find_package(CMocka 1.0.1)
    if(NOT CMOCKA_FOUND)
        message(STATUS "Disabling tests because of missing CMocka")
        set(ENABLE_TESTS OFF)
    endif()
endif()

if(ENABLE_PERF_TESTS)
    find_path(VALGRIND_INCLUDE_DIR
        NAMES
        valgrind/callgrind.h
        PATHS
        /usr/include
        /usr/local/include
        /opt/local/include
        /sw/include
        ${CMAKE_INCLUDE_PATH}
        ${CMAKE_INSTALL_PREFIX}/include)
    if(VALGRIND_INCLUDE_DIR)
        set(HAVE_CALLGRIND 1)
    else()
        message(STATUS "Disabling callgrind macros in performance tests because of missing valgrind headers")
    endif()
endif()

if(ENABLE_COVERAGE)
    gen_coverage_enable(${ENABLE_TESTS})
endif()

if ("${BUILD_TYPE_UPPER}" STREQUAL "DEBUG")
    # enable before adding tests to let them detect that format checking is available - one of the tests is format checking
    source_format_enable(0.77)
endif()

# generate files
configure_file(${PROJECT_SOURCE_DIR}/src/ly_config.h.in ${PROJECT_BINARY_DIR}/src/ly_config.h @ONLY)
configure_file(${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY)

# DOC-only target with no extra dependencies
if("${BUILD_TYPE_UPPER}" STREQUAL "DOCONLY")
    gen_doc("${doxy_files}" ${LIBYANG_VERSION} ${LIBYANG_DESCRIPTION} ${project_logo})
    return()
endif()

#
# targets
#

# link compat
use_compat()

# create static libyang library
if(NOT BUILD_SHARED_LIBS)
    add_definitions(-DSTATIC)

    # allow binaries compilation linking both static and dynamic libraries never linking static glibc
    #set(CMAKE_EXE_LINKER_FLAGS -static)

    # prefer static libraries
    set(CMAKE_FIND_LIBRARY_SUFFIXES .a;.so)
    set(CMAKE_LINK_SEARCH_START_STATIC TRUE)

    set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS)       # remove -Wl,-Bdynamic
    set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS)
    add_library(yang STATIC ${libsrc} ${compatsrc})
else()
    set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
    add_library(yangobj OBJECT ${libsrc} ${compatsrc})
    if(NOT WIN32)
        set_target_properties(yangobj PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")
    endif()
    target_compile_definitions(yangobj PRIVATE LIBYANG_BUILD)
    add_library(yang SHARED $<TARGET_OBJECTS:yangobj>)

    if(WIN32)
        find_package(dlfcn-win32 REQUIRED)
        set(CMAKE_DL_LIBS dlfcn-win32::dl)
    endif()

    #link dl
    target_link_libraries(yang ${CMAKE_DL_LIBS})
endif()

if(WIN32)
    find_path(DIRENT_INCLUDE_DIR NAMES dirent.h REQUIRED)
    message(STATUS "Found <dirent.h> at ${DIRENT_INCLUDE_DIR}")

    set(COMPAT_POSIX_INCLUDES
        ${CMAKE_CURRENT_SOURCE_DIR}/compat/posix-shims
        ${DIRENT_INCLUDE_DIR})

    if(TARGET yangobj)
        target_include_directories(yangobj PRIVATE ${COMPAT_POSIX_INCLUDES})
    endif()
    target_include_directories(yang PRIVATE ${COMPAT_POSIX_INCLUDES})
    include_directories(${COMPAT_POSIX_INCLUDES})

    find_package(pthreads REQUIRED)
    set(COMPAT_WIN_LIBRARIES PThreads4W::PThreads4W shlwapi.lib ws2_32)
    target_link_libraries(yang ${COMPAT_WIN_LIBRARIES})
endif()

set_target_properties(yang PROPERTIES VERSION ${LIBYANG_SOVERSION_FULL} SOVERSION ${LIBYANG_SOVERSION})

# link math
if(NOT WIN32)
    target_link_libraries(yang m)
endif()

# find pthreads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
target_link_libraries(yang ${CMAKE_THREAD_LIBS_INIT})

# find PCRE2 library
unset(PCRE2_LIBRARY CACHE)
find_package(PCRE2 10.21 REQUIRED)
include_directories(${PCRE2_INCLUDE_DIRS})
target_link_libraries(yang ${PCRE2_LIBRARIES})

# generated header list
foreach(h IN LISTS gen_headers)
    list(APPEND g_headers ${PROJECT_BINARY_DIR}/${h})
endforeach()

# install the modules
install(DIRECTORY "${PROJECT_SOURCE_DIR}/models/" DESTINATION ${YANG_MODULE_DIR} FILES_MATCHING PATTERN "*.yang")

# install all library files
install(TARGETS yang DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${headers} ${g_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

find_package(PkgConfig)
if(PKG_CONFIG_FOUND)
    # generate and install pkg-config file
    configure_file("libyang.pc.in" "libyang.pc" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libyang.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
    # check that pkg-config includes the used path
    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable pc_path pkg-config RESULT_VARIABLE RETURN OUTPUT_VARIABLE PC_PATH ERROR_QUIET)
    if(RETURN EQUAL 0)
    string(STRIP "${PC_PATH}" PC_PATH)
    set(PC_PATH "${PC_PATH}:$ENV{PKG_CONFIG_PATH}")
        string(REGEX MATCH "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig" SUBSTR "${PC_PATH}")
        string(LENGTH "${SUBSTR}" SUBSTR_LEN)
        if(SUBSTR_LEN EQUAL 0)
            message(WARNING "pkg-config will not detect the new package after installation, adjust PKG_CONFIG_PATH using \"export PKG_CONFIG_PATH=\${PKG_CONFIG_PATH}:${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/pkgconfig\".")
        endif()
    endif()
endif()

# tests
if(ENABLE_TESTS OR ENABLE_PERF_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(ENABLE_FUZZ_TARGETS)
    set(FUZZER "AFL" CACHE STRING "fuzzer type")
    if(FUZZER STREQUAL "LibFuzzer")
        if (NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
            message(FATAL_ERROR "LibFuzzer works only with clang")
        endif()
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,undefined -fno-omit-frame-pointer")
    endif()
endif()

# create coverage target for generating coverage reports
gen_coverage("utest_.*" "utest_.*_valgrind")

# tools - yanglint, yangre
if(ENABLE_TOOLS)
    add_subdirectory(tools)
endif()

# generate doxygen documentation for libyang API
gen_doc("${doxy_files}" ${LIBYANG_VERSION} ${LIBYANG_DESCRIPTION} ${project_logo})

# generate API/ABI report
if ("${BUILD_TYPE_UPPER}" STREQUAL "ABICHECK")
    lib_abi_check(yang "${headers}" ${LIBYANG_SOVERSION_FULL} dae82c1a652bdca0074544c62469a7f51d92c5e8)
endif()

# source code format target for Makefile
# - add it after tests which may also update list of sources to format
source_format(${format_sources})

# uninstall
add_custom_target(uninstall "${CMAKE_COMMAND}" -P "${CMAKE_MODULE_PATH}/uninstall.cmake")

# clean cmake cache
add_custom_target(cclean
        COMMAND make clean
        COMMAND find . -iname '*cmake*' -not -name CMakeLists.txt -not -path './CMakeModules*' -exec rm -rf {} +
        COMMAND rm -rf Makefile Doxyfile
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
