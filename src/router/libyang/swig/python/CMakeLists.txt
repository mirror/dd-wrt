set(PYTHON_SWIG_BINDING yang)
include_directories(${PYTHON_INCLUDE_PATH})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_SWIG_FLAGS "-c++")
set(CMAKE_SWIG_FLAGS "-I${PROJECT_SOURCE_DIR}")
set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

set_source_files_properties(${PYTHON_SWIG_BINDING}.i PROPERTIES CPLUSPLUS ON PREFIX "")

if(${CMAKE_VERSION} VERSION_LESS "3.8.0")
    swig_add_module(${PYTHON_SWIG_BINDING} python ${PYTHON_SWIG_BINDING}.i)
else()
    swig_add_library(${PYTHON_SWIG_BINDING} LANGUAGE python SOURCES ${PYTHON_SWIG_BINDING}.i)
endif()
swig_link_libraries(${PYTHON_SWIG_BINDING} ${PYTHON_LIBRARIES} libyang-cpp)

# Generate header with SWIG run-time functions
execute_process(COMMAND ${SWIG_EXECUTABLE} -python -external-runtime ${CMAKE_CURRENT_BINARY_DIR}/swigpyrun.h)

file(COPY "examples" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

execute_process(COMMAND ${PYTHON_EXECUTABLE} -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(plat_specific=True))"
                OUTPUT_VARIABLE PYTHON_MODULE_PATH
                OUTPUT_STRIP_TRAILING_WHITESPACE )

install( TARGETS _${PYTHON_SWIG_BINDING} DESTINATION ${PYTHON_MODULE_PATH})
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_SWIG_BINDING}.py" DESTINATION ${PYTHON_MODULE_PATH})
install( FILES "${CMAKE_CURRENT_BINARY_DIR}/swigpyrun.h" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/libyang)

if(ENABLE_BUILD_TESTS)
    set(PY2_SWIG_DIR ${CMAKE_CURRENT_BINARY_DIR})
    set(PY2_TEST_DIR ${PY2_SWIG_DIR}/tests)

    file(COPY "tests" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
    file(COPY "run_python_test.sh" DESTINATION "${PY2_SWIG_DIR}")

    macro(ADD_PYTHON_TEST TEST_NAME)
        add_test(NAME python_${TEST_NAME}
            COMMAND sh ${PY2_SWIG_DIR}/run_python_test.sh
            "${CMAKE_BINARY_DIR}/src:${CMAKE_BINARY_DIR}/tests:${PY2_TEST_DIR}"
            "${PY2_SWIG_DIR}:${PROJECT_SOURCE_DIR}/swig/python:${PYTHON_BUILD_DIR}"
            "${PYTHON_EXECUTABLE}"
            "${PY2_SWIG_DIR}/tests/${TEST_NAME}.py"
            "${CMAKE_BINARY_DIR}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            )
    endmacro(ADD_PYTHON_TEST)

    ADD_PYTHON_TEST(test_libyang)
    ADD_PYTHON_TEST(test_tree_data)
    ADD_PYTHON_TEST(test_tree_schema)

    add_custom_command(TARGET ${SWIG_MODULE_${PYTHON_SWIG_BINDING}_REAL_NAME} POST_BUILD
        COMMAND cp "${CMAKE_CURRENT_BINARY_DIR}/_${PYTHON_SWIG_BINDING}.so" ${PY2_SWIG_DIR}/tests
            COMMAND cp "${CMAKE_CURRENT_BINARY_DIR}/${PYTHON_SWIG_BINDING}.py" ${PY2_SWIG_DIR}/tests
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )

    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.py.in" "${CMAKE_CURRENT_BINARY_DIR}/tests/config.py" ESCAPE_QUOTES @ONLY)
endif()
