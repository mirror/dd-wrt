cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(libutp VERSION 3.4 LANGUAGES C CXX)

set(LIBUTP_STANDALONE_BUILD OFF)
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(LIBUTP_STANDALONE_BUILD ON)
endif()

option(LIBUTP_SHARED "${PROJECT_NAME}: Build shared library" ${BUILD_SHARED_LIBS})
option(LIBUTP_ENABLE_INSTALL "${PROJECT_NAME}: Enable install" ${LIBUTP_STANDALONE_BUILD})
option(LIBUTP_ENABLE_WERROR "${PROJECT_NAME}: Treat warnings as errors" ${LIBUTP_STANDALONE_BUILD})
option(LIBUTP_BUILD_PROGRAMS "${PROJECT_NAME}: Build programs" ${LIBUTP_STANDALONE_BUILD})

if(LIBUTP_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

if(LIBUTP_STANDALONE_BUILD)
    if(NOT CMAKE_C_STANDARD)
        set(CMAKE_C_STANDARD 99)
        set(CMAKE_C_STANDARD_REQUIRED ON)
        set(CMAKE_C_EXTENSIONS OFF)
    endif()

    if(NOT CMAKE_CXX_STANDARD)
        set(CMAKE_CXX_STANDARD 98)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()

    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(
            -Wall
            -Wextra
            -pedantic
            $<$<BOOL:${LIBUTP_ENABLE_WERROR}>:-Werror>)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(
            /W4
            /wd4244 # '...': conversion from '...' to '...', possible loss of data
            /wd4267 # '...': conversion from '...' to '...', possible loss of data
            $<$<BOOL:${LIBUTP_ENABLE_WERROR}>:/WX>)
    endif()
endif()

set(LIBUTP_LIBRARY_TYPE STATIC)
if(LIBUTP_SHARED)
    set(LIBUTP_LIBRARY_TYPE SHARED)
endif()

set(LIBUTP_ADD_SOURCES)
if(WIN32)
    list(APPEND LIBUTP_ADD_SOURCES
        libutp_inet_ntop.cpp
        libutp_inet_ntop.h)
endif()

add_library(libutp ${LIBUTP_LIBRARY_TYPE}
    include/libutp/utp.h
    include/libutp/utp_types.h
    utp_api.cpp
    utp_callbacks.cpp
    utp_callbacks.h
    utp_hash.cpp
    utp_hash.h
    utp_internal.cpp
    utp_internal.h
    utp_packedsockaddr.cpp
    utp_packedsockaddr.h
    utp_templates.h
    utp_utils.cpp
    utp_utils.h
    ${LIBUTP_ADD_SOURCES})

if(NOT LIBUTP_STANDALONE_BUILD)
    add_library(${PROJECT_NAME}::libutp ALIAS libutp)
endif()

set_target_properties(libutp
    PROPERTIES
        OUTPUT_NAME utp)

target_compile_definitions(libutp
    PRIVATE
        $<$<BOOL:${WIN32}>:_WINSOCK_DEPRECATED_NO_WARNINGS>
    PUBLIC
        $<$<BOOL:${WIN32}>:WIN32>
        $<$<NOT:$<BOOL:${WIN32}>>:POSIX>)

target_include_directories(libutp
    PRIVATE
        include/libutp
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

if(LIBUTP_ENABLE_INSTALL)
    install(
        TARGETS libutp
        EXPORT ${PROJECT_NAME}-targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
endif()

if(LIBUTP_BUILD_PROGRAMS AND NOT MSVC)
    add_executable(ucat
        ucat.c)

    if(NOT LIBB64_STANDALONE_BUILD)
        add_executable(${PROJECT_NAME}::ucat ALIAS ucat)
    endif()

    target_compile_definitions(ucat
        PRIVATE
            $<$<NOT:$<BOOL:${WIN32}>>:_POSIX_C_SOURCE=200112L>
            $<$<BOOL:${APPLE}>:_DARWIN_C_SOURCE>)

    target_link_libraries(ucat
        PRIVATE
            libutp)

    if(LIBUTP_ENABLE_INSTALL)
        install(
            TARGETS ucat
            EXPORT ${PROJECT_NAME}-targets
            DESTINATION ${CMAKE_INSTALL_BINDIR})
    endif()
endif()

if(LIBUTP_ENABLE_INSTALL)
    write_basic_package_version_file(
        ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

    configure_file(
        config.cmake.in
        ${PROJECT_NAME}-config.cmake
        @ONLY)

    install(
        EXPORT ${PROJECT_NAME}-targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        NAMESPACE ${PROJECT_NAME}::)

    install(
        FILES
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
            ${PROJECT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    install(
        FILES
            LICENSE
            README.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()
