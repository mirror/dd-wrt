option(DBUS_USE_WINE "set to 1 or ON to support running test cases with Wine" OFF)

if(DBUS_BUILD_TESTS AND CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
        find_file(WINE_EXECUTABLE
            NAMES wine
            PATHS /usr/bin /usr/local/bin
            NO_CMAKE_FIND_ROOT_PATH
        )
        find_file(BINFMT_WINE_SUPPORT_FILE
            NAMES DOSWin wine Wine windows Windows
            PATHS /proc/sys/fs/binfmt_misc
            NO_SYSTEM_PATH NO_CMAKE_FIND_ROOT_PATH
        )
        if(EXISTS BINFMT_WINE_SUPPORT_FILE)
            file(READ ${BINFMT_WINE_SUPPORT_FILE} CONTENT)
            if(${CONTENT} MATCHES "enabled")
                set(HAVE_BINFMT_WINE_SUPPORT 1)
            endif()
        endif()
        if(WINE_EXECUTABLE)
            list(APPEND FOOTNOTES "NOTE: The requirements to run cross compiled applications on your host system are achieved. You may run 'make check'.")
        endif()
        if(NOT WINE_EXECUTABLE)
            list(APPEND FOOTNOTES "NOTE: You may install the Windows emulator 'wine' to be able to run cross compiled test applications.")
        endif()
        if(NOT HAVE_BINFMT_WINE_SUPPORT)
            list(APPEND FOOTNOTES "NOTE: You may activate binfmt_misc support for wine to be able to run cross compiled test applications directly.")
        endif()
    else()
        list(APPEND FOOTNOTES "NOTE: You will not be able to run cross compiled applications on your host system.")
    endif()

    # setup z drive required by wine
    set(Z_DRIVE_IF_WINE "z:")
    if(DBUS_USE_WINE AND WINE_EXECUTABLE)
        set(TEST_WRAPPER "${WINE_EXECUTABLE}")
    endif()
endif()

macro(add_test_executable _target _source)
    set(_sources "${_source}")
    if(WIN32 AND NOT MSVC)
        # avoid triggering UAC
        add_uac_manifest(_sources)
    endif()
    add_executable(${_target} ${_sources})
    target_link_libraries(${_target} ${ARGN})
    if(CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # run tests with binfmt_misc
        if(HAVE_BINFMT_WINE_SUPPORT)
            add_test(NAME ${_target} COMMAND $<TARGET_FILE:${_target}> --tap)
        else()
            add_test(NAME ${_target} COMMAND ${TEST_WRAPPER} ${Z_DRIVE_IF_WINE}$<TARGET_FILE:${_target}> --tap)
        endif()
    else()
        add_test(
            NAME ${_target}
            COMMAND $<TARGET_FILE:${_target}> --tap
            WORKING_DIRECTORY ${DBUS_TEST_WORKING_DIR}
        )
    endif()
    set(_env)
    list(APPEND _env "DBUS_SESSION_BUS_ADDRESS=")
    list(APPEND _env "DBUS_FATAL_WARNINGS=1")
    list(APPEND _env "DBUS_TEST_DAEMON=${DBUS_TEST_DAEMON}")
    list(APPEND _env "DBUS_TEST_DATA=${DBUS_TEST_DATA}")
    list(APPEND _env "DBUS_TEST_DBUS_LAUNCH=${DBUS_TEST_DBUS_LAUNCH}")
    list(APPEND _env "DBUS_TEST_EXEC=${DBUS_TEST_EXEC}")
    list(APPEND _env "DBUS_TEST_HOMEDIR=${DBUS_TEST_HOMEDIR}")
    list(APPEND _env "DBUS_TEST_UNINSTALLED=1")
    set_tests_properties(${_target} PROPERTIES ENVIRONMENT "${_env}")
endmacro()

macro(add_helper_executable _target _source)
    set(_sources "${_source}")
    if(WIN32 AND NOT MSVC)
        # avoid triggering UAC
        add_uac_manifest(_sources)
    endif()
    add_executable(${_target} ${_sources})
    target_link_libraries(${_target} ${ARGN})
endmacro()

macro(add_session_test_executable _target _source)
    set(_sources "${_source}")
    if(WIN32 AND NOT MSVC)
        # avoid triggering UAC
        add_uac_manifest(_sources)
    endif()
    add_executable(${_target} ${_sources})
    target_link_libraries(${_target} ${ARGN})
    add_test(NAME ${_target}
        COMMAND
        ${TEST_WRAPPER}
        ${DBUS_TEST_RUN_SESSION}
        --config-file=${DBUS_TEST_DATA}/valid-config-files/tmp-session.conf
        --dbus-daemon=${DBUS_TEST_DAEMON}
        ${Z_DRIVE_IF_WINE}$<TARGET_FILE:${_target}>
        --tap
        WORKING_DIRECTORY ${DBUS_TEST_WORKING_DIR}
    )
    set(_env)
    list(APPEND _env "DBUS_SESSION_BUS_PID=")
    list(APPEND _env "DBUS_SESSION_BUS_ADDRESS=")
    list(APPEND _env "DBUS_FATAL_WARNINGS=1")
    list(APPEND _env "DBUS_TEST_DAEMON=${DBUS_TEST_DAEMON}")
    list(APPEND _env "DBUS_TEST_DATA=${DBUS_TEST_DATA}")
    list(APPEND _env "DBUS_TEST_HOMEDIR=${DBUS_TEST_HOMEDIR}")
    set_tests_properties(${_target} PROPERTIES ENVIRONMENT "${_env}")
endmacro()

#
# generate compiler flags from MSVC warning identifiers (e.g. '4114') or gcc warning keys (e.g. 'pointer-sign')
#
# @param target the variable name which will contain the warnings flags
# @param warnings a string with space delimited warnings
# @param disabled_warnings a string with space delimited disabled warnings
# @param error_warnings a string with space delimited warnings which should result into compile errors
#
macro(generate_warning_cflags target warnings disabled_warnings error_warnings)
    if(DEBUG_MACROS)
        message("generate_warning_cflags got: ${warnings} - ${disabled_warnings} - ${error_warnings}")
    endif()
    if(MSVC)
        # level 1 is default
        set(enabled_prefix "/w1")
        set(error_prefix "/we")
        set(disabled_prefix "/wd")
    else()
        set(enabled_prefix "-W")
        set(error_prefix "-Werror=")
        set(disabled_prefix "-Wno-")
    endif()

    set(temp)
    string(REPLACE " " ";" warnings_list "${warnings}")
    foreach(warning ${warnings_list})
        string(STRIP ${warning} _warning)
        if(_warning)
            set(temp "${temp} ${enabled_prefix}${_warning}")
        endif()
    endforeach()

    string(REPLACE " " ";" disabled_warnings_list "${disabled_warnings}")
    foreach(warning ${disabled_warnings_list})
        string(STRIP ${warning} _warning)
        if(_warning)
            set(temp "${temp} ${disabled_prefix}${_warning}")
        endif()
    endforeach()

    string(REPLACE " " ";" error_warnings_list "${error_warnings}")
    foreach(warning ${error_warnings_list})
        string(STRIP ${warning} _warning)
        if(_warning)
            set(temp "${temp} ${error_prefix}${_warning}")
        endif()
    endforeach()
    set(${target} "${temp}")
    if(DEBUG_MACROS)
        message("generate_warning_cflags return: ${${target}}")
    endif()
endmacro()

#
# Avoid triggering UAC
#
# This macro adds an rc file to _sources that is
# linked to a target and prevents UAC from making
# requests for administrator access.
#
macro(add_uac_manifest _sources)
    # 1 is the resource ID, ID_MANIFEST
    # 24 is the resource type, RT_MANIFEST
    # constants are used because of a bug in windres
    # see https://stackoverflow.com/questions/33000158/embed-manifest-file-to-require-administrator-execution-level-with-mingw32
    get_filename_component(UAC_FILE ${CMAKE_SOURCE_DIR}/tools/Win32.Manifest REALPATH)
    set(outfile ${CMAKE_BINARY_DIR}/disable-uac.rc)
    if(NOT EXISTS outfile)
        file(WRITE ${outfile} "1 24 \"${UAC_FILE}\"\n")
    endif()
    list(APPEND ${_sources} ${outfile})
endmacro()

macro(add_executable_version_info _sources _name)
    set(DBUS_VER_INTERNAL_NAME "${_name}")
    set(DBUS_VER_ORIGINAL_NAME "${DBUS_VER_INTERNAL_NAME}${CMAKE_EXECUTABLE_SUFFIX}")
    set(DBUS_VER_FILE_TYPE "VFT_APP")
    configure_file(${CMAKE_SOURCE_DIR}/dbus/versioninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-${DBUS_VER_INTERNAL_NAME}.rc)
    # version info and uac manifest can be combined in a binary because they use different resource types
    list(APPEND ${_sources} ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-${DBUS_VER_INTERNAL_NAME}.rc)
endmacro()

macro(add_library_version_info _sources _name)
    set(DBUS_VER_INTERNAL_NAME "${_name}")
    set(DBUS_VER_ORIGINAL_NAME "${DBUS_VER_INTERNAL_NAME}}${CMAKE_SHARED_LIBRARY_SUFFIX}")
    set(DBUS_VER_FILE_TYPE "VFT_DLL")
    configure_file(${CMAKE_SOURCE_DIR}/dbus/versioninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-${DBUS_VER_INTERNAL_NAME}.rc)
    # version info and uac manifest can be combined in a binary because they use different resource types
    list(APPEND ${_sources} ${CMAKE_CURRENT_BINARY_DIR}/versioninfo-${DBUS_VER_INTERNAL_NAME}.rc)
endmacro()
