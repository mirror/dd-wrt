project(libyang-cpp)

set(CMAKE_CXX_FLAGS         "${CMAKE_CXX_FLAGS} -Wall -Wextra -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_PACKAGE "-g -O2 -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG   "-g -Og")

# temporary bugfix
add_compile_options(-std=c++11)

# find SWIG package
if(GEN_LANGUAGE_BINDINGS)
    find_package(SWIG 3.0.12)
    if(NOT SWIG_FOUND)
        message(WARNING "SWIG library not found")
    else()
        include(${SWIG_USE_FILE})
    endif()
endif()

# find Python package
if(GEN_PYTHON_BINDINGS AND SWIG_FOUND)
    message(STATUS "Python version ${GEN_PYTHON_VERSION} was selected")
    unset(PYTHON_LIBRARY CACHE)
    unset(PYTHON_EXECUTABLE CACHE)
    unset(PYTHON_INCLUDE_DIR CACHE)
    unset(PYTHON_LIBRARY_DEBUG CACHE)
    if(${GEN_PYTHON_VERSION} STREQUAL "2")
        find_package(PythonInterp 2 REQUIRED)
        find_package(PythonLibs 2 REQUIRED)
        if(NOT PYTHONLIBS_FOUND)
            message(WARNING "Did not found Python version 2.x")
            message(STATUS "Libyang supports Python 2.x and Python 3.x")
        endif()
    elseif(${GEN_PYTHON_VERSION} STREQUAL "3")
        find_package(PythonInterp 3 REQUIRED)
        find_package(PythonLibs 3 REQUIRED)
        if(NOT PYTHONLIBS_FOUND)
            message(WARNING "Did not found Python version 3.x")
            message(STATUS "Libyang supports Python 2.x and Python 3.x")
        endif()
    else()
        message(WARNING "Libyang supports Python 2.x and Python 3.x")
    endif()
endif()

set(LIBYANG_CPP_SOURCES
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Xml.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Libyang.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Data.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Schema.cpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Internal.cpp)

set(LIBYANG_HPP_SOURCES
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Xml.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Libyang.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Data.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Tree_Schema.hpp
    ${CMAKE_SOURCE_DIR}/swig/cpp/src/Internal.hpp)

FUNCTION(PREPEND var prefix)
    SET(listVar "")
    FOREACH(f ${ARGN})
        LIST(APPEND listVar "${prefix}/${f}")
    ENDFOREACH(f)
    SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

PREPEND(abs_libsrc ${CMAKE_SOURCE_DIR} ${libsrc})

if(ENABLE_STATIC)
    add_library(libyang-cpp STATIC ${LIBYANG_CPP_SOURCES})
else(ENABLE_STATIC)
    add_library(libyang-cpp SHARED ${LIBYANG_CPP_SOURCES})
endif(ENABLE_STATIC)
SET_TARGET_PROPERTIES(libyang-cpp PROPERTIES
    VERSION ${LIBYANG_SOVERSION_FULL} SOVERSION ${LIBYANG_SOVERSION} PREFIX "")

# include custom Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

include_directories(${CMAKE_SOURCE_DIR})
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/cpp/src")
target_link_libraries(libyang-cpp yang)

if(PKG_CONFIG_FOUND)
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/libyang-cpp.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/libyang-cpp.pc" @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/libyang-cpp.pc" DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()

# install binary
install(TARGETS libyang-cpp DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${LIBYANG_HPP_SOURCES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

if (GEN_CPP_BINDINGS)
    add_subdirectory(cpp/examples)
    if (ENABLE_BUILD_TESTS)
        add_subdirectory(cpp/tests)
    endif()
endif()

if(ENABLE_STATIC AND PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND AND (${GEN_PYTHON_VERSION} STREQUAL "2" OR ${GEN_PYTHON_VERSION} STREQUAL "3"))
    message(WARNING "Can't create a static Python module")
elseif(PYTHONLIBS_FOUND AND PYTHONINTERP_FOUND AND (${GEN_PYTHON_VERSION} STREQUAL "2" OR ${GEN_PYTHON_VERSION} STREQUAL "3"))
    add_subdirectory(python)
endif()

if(NOT ENABLE_STATIC AND GEN_JAVASCRIPT_BINDINGS)
    message(WARNING "Can't create Javascript bindings with a shared library, please use -DENABLE_STATIC")
elseif(ENABLE_STATIC AND GEN_JAVASCRIPT_BINDINGS)
    add_subdirectory(javascript)
endif()

if(ENABLE_STATIC AND GEN_JAVA_BINDINGS)
    message(WARNING "Can't create a static Java module")
elseif(NOT ENABLE_STATIC AND GEN_JAVA_BINDINGS)
    add_subdirectory(java)
endif()
