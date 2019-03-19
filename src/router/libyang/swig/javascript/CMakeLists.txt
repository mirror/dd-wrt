cmake_minimum_required(VERSION 2.8.12)

find_package(Nodejs REQUIRED)
find_package(SWIG 3.0.12 REQUIRED)

add_compile_options(-std=gnu++11)

STRING(TOLOWER ${CMAKE_BUILD_TYPE} lower_type)
if(${lower_type} STREQUAL "release")
    set(JAVASCRIPT_BUILD_TYPE yang)
else()
    set(JAVASCRIPT_BUILD_TYPE yang-dev)
endif()

set(SWIG_DEST "${CMAKE_CURRENT_BINARY_DIR}")
file(MAKE_DIRECTORY ${SWIG_DEST})

set(V8_VERSION_HEX 0x0${V8_VERSION_MAJOR}${V8_VERSION_MINOR}${V8_VERSION_PATCH})
string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
while(V8_VERSION_HEX_length LESS 8)
    set(V8_VERSION_HEX "${V8_VERSION_HEX}0")
    string(LENGTH "${V8_VERSION_HEX}" V8_VERSION_HEX_length)
endwhile()

execute_process(COMMAND swig -c++ -javascript -node -DV8_VERSION=${V8_VERSION_HEX} -outcurrentdir -I${CMAKE_BINARY_DIR}/src -I${CMAKE_SOURCE_DIR}/src -I${CMAKE_SOURCE_DIR}/swig/cpp/src ${CMAKE_SOURCE_DIR}/swig/javascript/yang.i WORKING_DIRECTORY ${SWIG_DEST} RESULT_VARIABLE RET OUTPUT_VARIABLE OUT ERROR_VARIABLE ERROR)
if(NOT RET)
    function(PREPEND var prefix)
        set(listVar "")
        foreach(f ${ARGN})
            list(APPEND listVar "${prefix}/${f}")
        endforeach(f)
        set(${var} "${listVar}" PARENT_SCOPE)
    endfunction(PREPEND)

    macro(LIBYANG_CREATE_BINDING_GYP generated_file)
        set(libyang_LIB_SRCS_GYP "")
        PREPEND(libyang_NPM_SRCS "" ${libsrc})

        string(LENGTH "${CMAKE_SOURCE_DIR}/swig/cpp/" cpp_length)
        foreach(full_srcfile ${LIBYANG_CPP_SOURCES})
            string(SUBSTRING ${full_srcfile} ${cpp_length} -1 srcfile)
            set(libyang_LIB_SRCS_GYP "${libyang_LIB_SRCS_GYP}\n            '${srcfile}',")
        endforeach(full_srcfile)

        foreach(plugin ${USER_TYPE_LIST})
            set(libyang_LIB_SRCS_GYP "${libyang_LIB_SRCS_GYP}\n            'src/user_types/${plugin}.c',")
        endforeach(plugin)

        foreach(plugin ${EXTENSIONS_LIST})
            set(libyang_LIB_SRCS_GYP "${libyang_LIB_SRCS_GYP}\n            'src/extensions/${plugin}.c',")
        endforeach(plugin)

        foreach(srcfile ${libsrc})
            set(libyang_LIB_SRCS_GYP "${libyang_LIB_SRCS_GYP}\n            '${srcfile}',")
        endforeach(srcfile)

        configure_file(${CMAKE_SOURCE_DIR}/swig/javascript/${generated_file}.in ${SWIG_DEST}/${generated_file} @ONLY)
    endmacro(LIBYANG_CREATE_BINDING_GYP)
    LIBYANG_CREATE_BINDING_GYP(binding.gyp)

    file(COPY "${CMAKE_SOURCE_DIR}/src" DESTINATION ${SWIG_DEST})
    file(COPY "${CMAKE_SOURCE_DIR}/swig/cpp/src" DESTINATION ${SWIG_DEST})
    file(COPY "${CMAKE_BINARY_DIR}/src" DESTINATION "${SWIG_DEST}")
    file(COPY "${SWIG_DEST}/yang_wrap.cxx" DESTINATION "${SWIG_DEST}/src")
    file(COPY "${CMAKE_SOURCE_DIR}/models" DESTINATION ${SWIG_DEST})
    file(REMOVE "${SWIG_DEST}/yang_wrap.cxx")
    file(COPY "${CMAKE_SOURCE_DIR}/swig/javascript/README.md" DESTINATION "${SWIG_DEST}")
    file(COPY "${CMAKE_SOURCE_DIR}/swig/javascript/files" DESTINATION "${SWIG_DEST}")
    file(COPY "${CMAKE_SOURCE_DIR}/swig/javascript/test.js" DESTINATION "${SWIG_DEST}")
    configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/index.js.in" "${SWIG_DEST}/index.js" ESCAPE_QUOTES @ONLY)
    configure_file("${CMAKE_SOURCE_DIR}/swig/javascript/package.json.in" "${SWIG_DEST}/package.json" ESCAPE_QUOTES @ONLY)
else()
    message(${ERROR})
endif()

# remove javascript directory with "make clean" command.
set_property(DIRECTORY ${EXECUTABLE_OUTPUT_PATH} PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${SWIG_DEST}")
