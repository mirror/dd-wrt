cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(libb64 VERSION 2.0.0 LANGUAGES C)

set(LIBB64_STANDALONE_BUILD OFF)
if(PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(LIBB64_STANDALONE_BUILD ON)
endif()

option(LIBB64_SHARED "${PROJECT_NAME}: Build shared library" ${BUILD_SHARED_LIBS})
option(LIBB64_ENABLE_INSTALL "${PROJECT_NAME}: Enable install" ${LIBB64_STANDALONE_BUILD})
option(LIBB64_ENABLE_TESTS "${PROJECT_NAME}: Enable tests" ${LIBB64_STANDALONE_BUILD})
option(LIBB64_ENABLE_WERROR "${PROJECT_NAME}: Treat warnings as errors" ${LIBB64_STANDALONE_BUILD})
option(LIBB64_BUILD_PROGRAMS "${PROJECT_NAME}: Build programs" ${LIBB64_STANDALONE_BUILD})
option(LIBB64_BUILD_EXAMPLES "${PROJECT_NAME}: Build examples" ${LIBB64_STANDALONE_BUILD})

if(LIBB64_ENABLE_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)
endif()

if(LIBB64_ENABLE_TESTS)
    enable_testing()
endif()

if(LIBB64_STANDALONE_BUILD)
    if(NOT CMAKE_C_STANDARD)
        set(CMAKE_C_STANDARD 90)
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
            $<$<BOOL:${LIBB64_ENABLE_WERROR}>:-Werror>)
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC")
        add_compile_options(
            /W4
            $<$<BOOL:${LIBB64_ENABLE_WERROR}>:/WX>)
    endif()
endif()

set(LIBB64_LIBRARY_TYPE STATIC)
if(LIBB64_SHARED)
    set(LIBB64_LIBRARY_TYPE SHARED)
endif()

add_subdirectory(src)

if(LIBB64_BUILD_PROGRAMS)
    add_subdirectory(base64)
endif()

if(LIBB64_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

if(LIBB64_ENABLE_INSTALL)
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
            AUTHORS.md
            BENCHMARKS.md
            CHANGELOG.md
            INSTALL.md
            LICENSE.md
            README.md
            TODO.md
        DESTINATION ${CMAKE_INSTALL_DOCDIR})
endif()
